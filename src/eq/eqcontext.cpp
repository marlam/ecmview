/*
 * Copyright (C) 2008, 2009, 2010, 2011, 2012, 2012
 * Computer Graphics Group, University of Siegen, Germany.
 * Written by Martin Lambers <martin.lambers@uni-siegen.de>.
 * See http://www.cg.informatik.uni-siegen.de/ for contact information.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <eq/eq.h>

#include <GL/glew.h>

#include "dbg.h"
#include "thread.h"
#include "msg.h"
#include "sys.h"

#include "glvm.h"
#include "renderer.h"
#include "navigator.h"

#include "tracking.h"
#include "tracking-eqevent.h"
#include "tracking-dtrack.h"
#include "warping.h"

#include "eqcontext.h"

using namespace glvm;


class eq_init_data : public co::Object
{
public:
    eq::uint128_t frame_data_id;

    eq_init_data()
    {
    }

protected:
    virtual ChangeType getChangeType() const
    {
        return co::Object::STATIC;
    }

    virtual void getInstanceData(co::DataOStream& os)
    {
        std::ostringstream oss;
        s11n::save(oss, frame_data_id.high());
        s11n::save(oss, frame_data_id.low());
        os << oss.str();
    }

    virtual void applyInstanceData(co::DataIStream& is)
    {
        std::string s;
        is >> s;
        std::istringstream iss(s);
        s11n::load(iss, frame_data_id.high());
        s11n::load(iss, frame_data_id.low());
    }
};

class eq_frame_data : public co::Object
{
public:
    class state state;

    eq_frame_data()
    {
    }

protected:
    virtual ChangeType getChangeType() const
    {
        return co::Object::INSTANCE;
    }

    virtual void getInstanceData(co::DataOStream &os)
    {
        std::ostringstream oss;
        s11n::save(oss, state);
        os << oss.str();
    }

    virtual void applyInstanceData(co::DataIStream &is)
    {
        std::string s;
        is >> s;
        std::istringstream iss(s);
        s11n::load(iss, state);
    }
};

class eq_config : public eq::Config
{
private:
    eq_init_data _eq_init_data;         // Master eq_init_data instance
    eq_frame_data _eq_frame_data;       // Master eq_frame_data instance
    navigator _navigator;
    TrackingDriverDTrack *_tracking_driver_dtrack;
    TrackingDriverEQEvent *_tracking_driver_eqevent;
    Tracking *_tracking;
    int _flystick_handle;
    int _viewer_handle;
    bool _quit_request;

public:
    eq_config(eq::ServerPtr parent) : eq::Config(parent),
        _eq_init_data(), _eq_frame_data(),
        _navigator(),
        _tracking_driver_dtrack(NULL),
        _tracking_driver_eqevent(NULL),
        _tracking(NULL),
        _quit_request(false)
    {
    }

    ~eq_config()
    {
        delete _tracking;
        delete _tracking_driver_dtrack;
        delete _tracking_driver_eqevent;
    }

    bool init(const class state& state)
    {
        msg::set_level(state.msg_level);
        // Initialize state
        _eq_frame_data.state = state;
        // Register master instances
        registerObject(&_eq_frame_data);
        _eq_init_data.frame_data_id = _eq_frame_data.getID();
        registerObject(&_eq_init_data);
        // Initialize tracking
        if (_eq_frame_data.state.tracking != 0) {
            if (_eq_frame_data.state.tracking == 1) {
                _tracking_driver_eqevent = new TrackingDriverEQEvent();
                _tracking = new Tracking(_tracking_driver_eqevent);
            } else {
                _tracking_driver_dtrack = new TrackingDriverDTrack();
                _tracking = new Tracking(_tracking_driver_dtrack);
            }
            _flystick_handle = _tracking->track(Tracking::FLYSTICK, 0);
            _tracking->set_auto_repeat(_flystick_handle, 1);
            _tracking->set_auto_repeat(_flystick_handle, 2);
            _tracking->set_auto_repeat(_flystick_handle, 3);
            _tracking->set_auto_repeat(_flystick_handle, 4);
            _viewer_handle = _tracking->track(Tracking::BODY, 0);
        }
        return eq::Config::init(_eq_init_data.getID());
    }

    void set_state(const class state& state)
    {
        _eq_frame_data.state = state;
    }

    void get_state(class state* state)
    {
        *state = _eq_frame_data.state;
    }

    virtual bool exit()
    {
        bool ret = eq::Config::exit();
        // Deregister master instances
        deregisterObject(&_eq_init_data);
        deregisterObject(&_eq_frame_data);
        return ret;
    }

    virtual uint32_t startFrame()
    {
        if (_quit_request) {
            this->exit();
            return 0;
        }

        class state& state = _eq_frame_data.state;

        // Tracking
        if (state.tracking) {
            _tracking->update();
            if (_tracking->up_to_date(_viewer_handle)) {
                state.tracker_pos = _tracking->pos(_viewer_handle);
                state.tracker_rot = toQuat(_tracking->rot(_viewer_handle));
            }
            if (_tracking->up_to_date(_flystick_handle)) {
                const float dist_to_inner_bounding_sphere = length(state.viewer_pos) - state.inner_bounding_sphere_radius;
                const float joy_x = _tracking->joystick_x(_flystick_handle);
                const float joy_y = _tracking->joystick_y(_flystick_handle);
                const mat3 flystick_rot = mat3(toMat3(state.viewer_rot)) * _tracking->rot(_flystick_handle);
                const vec3 flystick_ahead = flystick_rot * vec3(0.0, 0.0, -1.0);
                const vec3 flystick_up = flystick_rot * vec3(0.0, 1.0, 0.0);
                const vec3 flystick_right = flystick_rot * vec3(1.0, 0.0, 0.0);
                if (_tracking->button(_flystick_handle, 4)) {
                    // Look towards planet
                    state.viewer_rot = toQuat(dvec3(0.0, 0.0, -1.0), -state.viewer_pos)
                        * toQuat(dvec3(1.0, 0.0, 0.0), dvec3(0.0, 1.0, 0.0));
                }
                if (_tracking->button(_flystick_handle, 0)) {
                    // Rotate planet
                    const vec3 normalized_head_pos = vec3(normalize(-state.viewer_pos));
                    const vec3 axis_y = cross(normalized_head_pos, flystick_up);
                    const float angle_y = (joy_y * dist_to_inner_bounding_sphere) / (state.inner_bounding_sphere_radius * 80.0f);
                    const vec3 axis_x = cross(normalized_head_pos, flystick_right);
                    const float angle_x = (joy_x * dist_to_inner_bounding_sphere) / (state.inner_bounding_sphere_radius * 80.0f);
                    const dquat rot = dquat(toQuat(angle_y, axis_y) * toQuat(angle_x, axis_x));
                    state.viewer_pos = -rot * state.viewer_pos;
                    state.viewer_rot = -rot * state.viewer_rot;
                }
                if (!_tracking->button(_flystick_handle, 0)
                        && !_tracking->button(_flystick_handle, 1)
                        && !_tracking->button(_flystick_handle, 2)
                        && !_tracking->button(_flystick_handle, 3)
                        && !_tracking->button(_flystick_handle, 4)) {
                    // Move in flystick direction
                    state.viewer_pos += dvec3(flystick_ahead * (joy_y * dist_to_inner_bounding_sphere / 40.0f)
                            + flystick_right * (joy_x * dist_to_inner_bounding_sphere / 40.0f));
                }
            }
        }

        // Set viewer head position and orientation
        mat4 head_tracking_matrix = toMat4(state.tracker_rot);
        translation(head_tracking_matrix) = state.tracker_pos;
        eq::Matrix4f eq_head_matrix;
        for (int i = 0; i < 16; i++) {
            eq_head_matrix.array[i] = head_tracking_matrix.vl[i];
        }
        getObservers().at(0)->setHeadMatrix(eq_head_matrix);
        
        // Commit new version of updated frame data
        const eq::uint128_t version = _eq_frame_data.commit();

        // Start this frame with the committed frame data
        return eq::Config::startFrame(version);
    }

    virtual bool handleEvent(const eq::ConfigEvent* event)
    {
        if (eq::Config::handleEvent(event)) {
            return true;
        }
        if (_tracking_driver_eqevent && _tracking_driver_eqevent->handle_event(event)) {
            return true;
        }
        bool event_requires_redraw = false;
        /* Unfortunately Equalizer events do not carry modifier state, so we cannot
         * implement the Shift, Ctrl and Meta functionality that the GUI has. */
        class state& state = _eq_frame_data.state;
        if (event->data.type == eq::Event::KEY_PRESS) {
            switch (event->data.keyPress.key) {
            case 'q':
                _quit_request = true;
                event_requires_redraw = true;
                break;
            }
        } else if (event->data.type == eq::Event::CHANNEL_POINTER_MOTION
                || event->data.type == eq::Event::CHANNEL_POINTER_BUTTON_PRESS) {
            ivec4 viewport = ivec4(
                    event->data.context.pvp.x, event->data.context.pvp.y,
                    event->data.context.pvp.w, event->data.context.pvp.h);
            _navigator.set_viewport(viewport);
            if (event->data.type == eq::Event::CHANNEL_POINTER_BUTTON_PRESS) {
                ivec2 pos = ivec2(event->data.pointerButtonRelease.x, event->data.pointerButtonRelease.y);
                if (event->data.pointerButtonRelease.buttons & eq::PTR_BUTTON1) {
                    _navigator.planet_rot_start(pos, state.inner_bounding_sphere_radius);
                    event_requires_redraw = true;
                } else if (event->data.pointerButtonRelease.buttons & eq::PTR_BUTTON2) {
                    _navigator.viewer_rot_start(pos);
                    event_requires_redraw = true;
                } else if (event->data.pointerButtonRelease.buttons & eq::PTR_BUTTON3) {
                    _navigator.planet_distchange_start(pos, state.viewer_pos, state.inner_bounding_sphere_radius);
                    event_requires_redraw = true;
                }
            } else {
                ivec2 pos = ivec2(event->data.pointerMotion.x, event->data.pointerMotion.y);
                if (event->data.pointerButtonRelease.buttons & eq::PTR_BUTTON1) {
                    _navigator.planet_rot(1.0, pos, state.viewer_pos, state.viewer_rot);
                    event_requires_redraw = true;
                } else if (event->data.pointerButtonRelease.buttons & eq::PTR_BUTTON2) {
                    _navigator.viewer_rot(1.0, pos, state.viewer_rot);
                    event_requires_redraw = true;
                } else if (event->data.pointerButtonRelease.buttons & eq::PTR_BUTTON3) {
                    _navigator.planet_distchange(1.0, pos, state.viewer_pos);
                    event_requires_redraw = true;
                }
            }
        } else if (event->data.type == eq::Event::WINDOW_POINTER_WHEEL) {
            _navigator.viewer_shift(100.0, event->data.pointerWheel.xAxis * 2.0 * const_pi<double>() / 24.0,
                    state.inner_bounding_sphere_radius, state.viewer_rot,
                    state.viewer_pos);
            event_requires_redraw = true;
        }
        return event_requires_redraw;
    }
};

class eq_node : public eq::Node
{
public:
    class eq_init_data eq_init_data;
    class eq_frame_data eq_frame_data;
    int per_node_maintenance_assigned;
    volatile int per_node_maintenance_finished;
    class quad_disk_cache* quad_disk_cache;
    class quad_disk_cache_checkers* quad_disk_cache_checkers;
    class quad_disk_cache_fetchers* quad_disk_cache_fetchers;
    class quad_mem_cache* quad_mem_cache;
    class quad_mem_cache_loaders* quad_mem_cache_loaders;
    class quad_metadata_cache* quad_metadata_cache;
    class quad_base_data_mem_cache* quad_base_data_mem_cache;
    class quad_base_data_mem_cache_computers* quad_base_data_mem_cache_computers;

public:
    eq_node(eq::Config* parent) : eq::Node(parent)
    {
        quad_disk_cache = NULL;
        quad_disk_cache_checkers = NULL;
        quad_disk_cache_fetchers = NULL;
        quad_mem_cache = NULL;
        quad_mem_cache_loaders = NULL;
        quad_metadata_cache = NULL;
        quad_base_data_mem_cache = NULL;
        quad_base_data_mem_cache_computers = NULL;
    }

protected:
    virtual bool configInit(const eq::uint128_t& init_id)
    {
        if (!eq::Node::configInit(init_id)) {
            return false;
        }
        // Map our InitData instance to the master instance
        eq_config* config = static_cast<eq_config*>(getConfig());
        if (!config->mapObject(&eq_init_data, init_id)) {
            return false;
        }
        // Map our FrameData instance to the master instance
        if (!config->mapObject(&eq_frame_data, eq_init_data.frame_data_id)) {
            return false;
        }
        // Initialize
        const class state& state = eq_frame_data.state;
        msg::set_level(state.msg_level);
        assert(!state.app_id.empty());
        assert(!state.cache_dir.empty());
        quad_disk_cache = new class quad_disk_cache(state.app_id, state.cache_dir);
        quad_disk_cache_checkers = new class quad_disk_cache_checkers(16, quad_disk_cache);
        quad_disk_cache_fetchers = new class quad_disk_cache_fetchers(16, quad_disk_cache);
        quad_mem_cache = new class quad_mem_cache();
        quad_mem_cache_loaders = new class quad_mem_cache_loaders(
                min(255, sys::processors() * 3 / 2 + 1), quad_mem_cache);
        quad_metadata_cache = new class quad_metadata_cache();
        quad_base_data_mem_cache = new class quad_base_data_mem_cache();
        quad_base_data_mem_cache_computers = new class quad_base_data_mem_cache_computers(
                min(255, sys::processors() * 3 / 2 + 1), quad_base_data_mem_cache);
        return true;
    }

    virtual bool configExit()
    {
        eq::Config* config = getConfig();
        // Unmap our FrameData instance
        config->unmapObject(&eq_frame_data);
        // Unmap our InitData instance
        config->unmapObject(&eq_init_data);
        // Cleanup
        delete quad_disk_cache;
        delete quad_disk_cache_checkers;
        delete quad_disk_cache_fetchers;
        delete quad_mem_cache;
        delete quad_mem_cache_loaders;
        delete quad_metadata_cache;
        delete quad_base_data_mem_cache;
        delete quad_base_data_mem_cache_computers;
        return eq::Node::configExit();
    }

    virtual void frameStart(const eq::uint128_t& frame_id, const uint32_t frame_number)
    {
        per_node_maintenance_assigned = 0;
        per_node_maintenance_finished = 0;
        // Update our frame data
        eq_frame_data.sync(frame_id);
        startFrame(frame_number);
    }
};

class eq_pipe : public eq::Pipe
{
public:
    eq_pipe(eq::Node* parent) : eq::Pipe(parent)
    {
    }
};

class eq_window : public eq::Window
{
public:
    class warping* warping;
    int per_glcontext_maintenance_assigned;
    volatile int per_glcontext_maintenance_finished;
    class quad_gpu_cache* quad_gpu_cache;
    class quad_base_data_gpu_cache* quad_base_data_gpu_cache;
    class quad_tex_pool* quad_tex_pool;

    eq_window(eq::Pipe* parent) : eq::Window(parent)
    {
        warping = NULL;
        quad_gpu_cache = NULL;
        quad_base_data_gpu_cache = NULL;
        quad_tex_pool = NULL;
    }
    
protected:
    virtual bool configInitGL(const eq::uint128_t& init_id)
    {
        if (!eq::Window::configInitGL(init_id))
            return false;

        // Initialize
        warping = new class warping(getName(), getPixelViewport());
        quad_gpu_cache = new class quad_gpu_cache();
        quad_base_data_gpu_cache = new class quad_base_data_gpu_cache();
        quad_tex_pool = new class quad_tex_pool();
        try {
            warping->init_gl();
            quad_tex_pool->init_gl();
        }
        catch (...) {
            msg::err("configInitGL() failed");
            return false;
        }

        // Disable some things that Equalizer seems to enable for some reason.
        glDisable(GL_LIGHTING);

        return true;
    }

    virtual bool configExitGL()
    {
        delete quad_gpu_cache;
        delete quad_base_data_gpu_cache;
        try {
            warping->exit_gl();
            quad_tex_pool->exit_gl();
        }
        catch (...) {
            msg::err("configExitGL() failed");
            return false;
        }
        delete quad_tex_pool;
        return eq::Window::configExitGL();
    }

    virtual void frameStart(const eq::uint128_t& /* frame_id */, const uint32_t frame_number)
    {
        per_glcontext_maintenance_assigned = 0;
        per_glcontext_maintenance_finished = 0;
        startFrame(frame_number);
    }
};

class eq_channel : public eq::Channel, public renderer_context
{
public:
    class renderer* renderer;

    eq_channel(eq::Window *parent) : eq::Channel(parent), renderer_context()
    {
        renderer = NULL;
    }

protected:

    virtual bool configInit(const eq::uint128_t& init_id)
    {
        if (!eq::Channel::configInit(init_id))
            return false;
        renderer = new class renderer(*this);
        getWindow()->makeCurrent();
        /*
        std::vector<std::string> missing_gl_features = initialize_gl();
        if (!missing_gl_features.empty()) {
            msg::err("The following OpenGL features are not available:");
            for (auto it = missing_gl_features.begin(); it != missing_gl_features.end(); it++) {
                msg::err("%s", std::string(*it).c_str());
            }
            return false;
        }
        */
        try {
            renderer->init_gl();
        }
        catch (...) {
            msg::err("eq_channel::configInit() failed");
            return false;
        }
        return true;
    }

    virtual bool configExit()
    {
        getWindow()->makeCurrent();
        try {
            renderer->exit_gl();
        }
        catch (...) {
            msg::err("eq_channel::configExit() failed");
            return false;
        }
        delete renderer;
        return eq::Channel::configExit();
    }

    virtual void frameDraw(const eq::uint128_t& frame_id)
    {
        // Let Equalizer initialize some stuff
        eq::Channel::frameDraw(frame_id);
        // Set up warping
        class eq_window* eq_window = static_cast<class eq_window*>(getWindow());
        eq_window->warping->setup();
        // Render
        const eq::PixelViewport& eq_viewport = getPixelViewport();
        const ivec4 viewport(eq_viewport.x, eq_viewport.y, eq_viewport.w, eq_viewport.h);
        const eq::Frustumf& eq_frustum = getFrustum();
        const dfrust frustum(eq_frustum.left(), eq_frustum.right(),
                eq_frustum.bottom(), eq_frustum.top(),
                eq_frustum.near_plane(), eq_frustum.far_plane());
        const eq::Matrix4f eq_head_transform = getHeadTransform();
        const dmat4 head_transform(mat4(eq_head_transform.array));
        try {
            renderer->render(viewport, frustum, head_transform);
        }
        catch (std::exception& e) {
            msg::err("Cannot render: %s", e.what());
            abort();
        }
        // Apply warping
        eq_window->warping->apply(state().tracker_pos);
        // Draw Statistics
        if (state().renderer.statistics_overlay) {
            drawStatistics();
        }
    }

    /*
     * The following renderer_context interfaces are only used by the renderer, not by
     * the application. The application only knows the eq_context renderer_context.
     */
    virtual GLEWContext* glewGetContext()
    {
        return const_cast<GLEWContext*>(getWindow()->glewGetContext());
    }
    virtual void init_gl() {}   // dummy
    virtual void exit_gl() {}   // dummy
    virtual void render() {}    // dummy
    virtual const class state& state() const
    {
        return static_cast<const eq_node*>(getNode())->eq_frame_data.state;
    }
    virtual bool start_per_node_maintenance()
    {
        eq_node* node = static_cast<eq_node*>(getNode());
        if (atomic::bool_compare_and_swap(&(node->per_node_maintenance_assigned), 0, 1)) {
            return true;
        } else {
            while (!(node->per_node_maintenance_finished)) {
                // busy loop until maintenance thread called finish_per_node_maintenance()
                sys::sched_yield();
            }
            return false;
        }
    }
    virtual void finish_per_node_maintenance()
    {
        static_cast<eq_node*>(getNode())->per_node_maintenance_finished = 1;
    }
    virtual bool start_per_glcontext_maintenance()
    {
        eq_window* window = static_cast<eq_window*>(getWindow());
        if (atomic::bool_compare_and_swap(&(window->per_glcontext_maintenance_assigned), 0, 1)) {
            return true;
        } else {
            while (!(window->per_glcontext_maintenance_finished)) {
                // busy loop until maintenance thread called finish_per_glcontext_maintenance()
                sys::sched_yield();
            }
            return false;
        }
    }
    virtual void finish_per_glcontext_maintenance()
    {
        static_cast<eq_window*>(getWindow())->per_glcontext_maintenance_finished = 1;
    }
    virtual class quad_disk_cache* quad_disk_cache()
    {
        return static_cast<eq_node*>(getNode())->quad_disk_cache;
    }
    virtual class quad_disk_cache_checkers* quad_disk_cache_checkers()
    {
        return static_cast<eq_node*>(getNode())->quad_disk_cache_checkers;
    }
    virtual class quad_disk_cache_fetchers* quad_disk_cache_fetchers()
    {
        return static_cast<eq_node*>(getNode())->quad_disk_cache_fetchers;
    }
    virtual class quad_mem_cache* quad_mem_cache()
    {
        return static_cast<eq_node*>(getNode())->quad_mem_cache;
    }
    virtual class quad_mem_cache_loaders* quad_mem_cache_loaders()
    {
        return static_cast<eq_node*>(getNode())->quad_mem_cache_loaders;
    }
    virtual class quad_gpu_cache* quad_gpu_cache()
    {
        return static_cast<eq_window*>(getWindow())->quad_gpu_cache;
    }
    virtual class quad_metadata_cache* quad_metadata_cache()
    {
        return static_cast<eq_node*>(getNode())->quad_metadata_cache;
    }
    virtual class quad_base_data_mem_cache* quad_base_data_mem_cache()
    {
        return static_cast<eq_node*>(getNode())->quad_base_data_mem_cache;
    }
    virtual class quad_base_data_mem_cache_computers* quad_base_data_mem_cache_computers()
    {
        return static_cast<eq_node*>(getNode())->quad_base_data_mem_cache_computers;
    }
    virtual class quad_base_data_gpu_cache* quad_base_data_gpu_cache()
    {
        return static_cast<eq_window*>(getWindow())->quad_base_data_gpu_cache;
    }
    virtual class quad_tex_pool* quad_tex_pool()
    {
        return static_cast<eq_window*>(getWindow())->quad_tex_pool;
    }
};

class eq_node_factory : public eq::NodeFactory
{
public:
    virtual eq::Config* createConfig(eq::ServerPtr parent)
    {
        return new eq_config(parent);
    }

    virtual eq::Node *createNode(eq::Config *parent)
    {
        return new eq_node(parent);
    }

    virtual eq::Pipe *createPipe(eq::Node *parent)
    {
        return new eq_pipe(parent);
    }

    virtual eq::Window* createWindow(eq::Pipe *parent)
    {
        return new eq_window(parent);
    }

    virtual eq::Channel* createChannel(eq::Window *parent)
    {
        return new eq_channel(parent);
    }
};

eq_context::eq_context(int *argc, char *argv[], class state* master_state) :
    _valid(false), _master_state(master_state)
{
    /* Initialize Equalizer */
    _eq_node_factory = new eq_node_factory;
    if (!eq::init(*argc, argv, _eq_node_factory)) {
        throw exc("Equalizer initialization failed.");
    }
    /* Get a configuration */
    _eq_config = static_cast<eq_config*>(eq::getConfig(*argc, argv));
    // The following code is only executed on the application node because
    // eq::getConfig() does not return on other nodes.
    if (!_eq_config) {
        throw exc("Cannot get Equalizer configuration.");
    }
    if (!_eq_config->init(*_master_state)) {
        throw exc("Cannot initialize Equalizer configuration.");
    }
    _valid = true;
}

eq_context::~eq_context()
{
    if (_valid && _eq_config) {
        _eq_config->exit();
        eq::releaseConfig(_eq_config);
        eq::exit();
    }
    delete _eq_node_factory;
}

void eq_context::quit()
{
    if (is_running()) {
        _eq_config->exit();
    }
}

bool eq_context::is_running()
{
    return _valid && _eq_config->isRunning();
}

/*
 * The following renderer_context interfaces are only used by the application, not by
 * the renderer. The renderer only knows the eq_channel renderer_context.
 */

GLEWContext* eq_context::glewGetContext()
{
    return NULL;
}

void eq_context::init_gl()
{
}

void eq_context::exit_gl()
{
}

void eq_context::render()
{
    if (_valid) {
        _eq_config->set_state(*_master_state);
        _eq_config->startFrame();
        _eq_config->finishFrame();
        _eq_config->get_state(_master_state);
        tick();
    }
}

const class state& eq_context::state() const
{
    return *_master_state;
}

bool eq_context::start_per_node_maintenance()
{
    return false;
}

void eq_context::finish_per_node_maintenance()
{
}

bool eq_context::start_per_glcontext_maintenance()
{
    return false;
}

void eq_context::finish_per_glcontext_maintenance()
{
}

class quad_disk_cache* eq_context::quad_disk_cache()
{
    return NULL;
}

class quad_disk_cache_checkers* eq_context::quad_disk_cache_checkers()
{
    return NULL;
}

class quad_disk_cache_fetchers* eq_context::quad_disk_cache_fetchers()
{
    return NULL;
}

class quad_mem_cache* eq_context::quad_mem_cache()
{
    return NULL;
}

class quad_mem_cache_loaders* eq_context::quad_mem_cache_loaders()
{
    return NULL;
}

class quad_gpu_cache* eq_context::quad_gpu_cache()
{
    return NULL;
}

class quad_metadata_cache* eq_context::quad_metadata_cache()
{
    return NULL;
}

class quad_base_data_mem_cache* eq_context::quad_base_data_mem_cache()
{
    return NULL;
}

class quad_base_data_mem_cache_computers* eq_context::quad_base_data_mem_cache_computers()
{
    return NULL;
}

class quad_base_data_gpu_cache* eq_context::quad_base_data_gpu_cache()
{
    return NULL;
}

class quad_tex_pool* eq_context::quad_tex_pool()
{
    return NULL;
}
