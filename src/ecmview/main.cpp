/*
 * Copyright (C) 2011, 2012
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

#include <cstring>
#include <cstdlib>
#include <limits>
#include <locale.h>

#if (defined _WIN32 || defined __WIN32__) && !defined __CYGWIN__
# include <windows.h>
# include <fcntl.h>
#endif

#include "gettext.h"
#define _(string) gettext(string)

#include "dbg.h"
#include "exc.h"
#include "fio.h"
#include "msg.h"
#include "opt.h"

#include "uuid.h"
#include "state.h"
#include "guimain.h"
#if HAVE_LIBEQUALIZER
# include "eqcontext.h"
#endif


int main(int argc, char *argv[])
{
#if W32
    _fmode = _O_BINARY;         // Open all files in binary mode by default
#endif

    /* Initialization: gettext */
    /*
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, localedir());
    textdomain(PACKAGE);
    */

    /* Initialization: messages */
    char *program_name = strrchr(argv[0], '/');
    program_name = program_name ? program_name + 1 : argv[0];
#ifdef NDEBUG
    msg::set_level(msg::INF);
#else
    msg::set_level(msg::DBG);
#endif
    msg::set_program_name(program_name);
    msg::set_columns_from_env();
    dbg::init_crashhandler();

    /* Initialization: Qt */
    gui_initialize(&argc, argv);

    /* Command line handling */
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    opt::info version("version", '\0', opt::optional);
    options.push_back(&version);
    opt::string config("config", '\0', opt::optional);
    options.push_back(&config);
    opt::string cache("cache", '\0', opt::optional);
    options.push_back(&cache);
    opt::string state("state", 's', opt::optional);
    options.push_back(&state);
    std::vector<std::string> tracking_values;
    tracking_values.push_back("off");
    tracking_values.push_back("eqevent");
    tracking_values.push_back("dtrack");
    opt::val<std::string> tracking("tracking", '\0', opt::optional, tracking_values, "off");
    options.push_back(&tracking);
    opt::flag equalizer("equalizer", 'E', opt::optional);
    options.push_back(&equalizer);
    // Accept some Equalizer options. These are passed to Equalizer for interpretation.
    opt::val<std::string> eq_server("eq-server", '\0', opt::optional);
    options.push_back(&eq_server);
    opt::val<std::string> eq_config("eq-config", '\0', opt::optional);
    options.push_back(&eq_config);
    opt::val<std::string> eq_listen("eq-listen", '\0', opt::optional);
    options.push_back(&eq_listen);
    opt::val<std::string> eq_logfile("eq-logfile", '\0', opt::optional);
    options.push_back(&eq_logfile);
    opt::val<std::string> eq_render_client("eq-render-client", '\0', opt::optional);
    options.push_back(&eq_render_client);

    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, 0, -1, arguments)) {
        return 1;
    }
    if (version.value()) {
        msg::req_txt("%s version %s on %s\n"
                "Copyright (C) 2012 Computer Graphics Group, University of Siegen, Germany.\n"
                "Written by Martin Lambers <martin.lambers@uni-siegen.de>.\n"
                "See http://www.cg.informatik.uni-siegen.de/ for contact information.\n"
                "This is free software. You may redistribute copies of it under the terms of "
                "the GNU General Public License.\n"
                "There is NO WARRANTY, to the extent permitted by law.\n"
                "See %s for more information on this software.",
                PACKAGE_NAME, PACKAGE_VERSION, PLATFORM, PACKAGE_URL);
    }
    if (help.value()) {
        msg::req_txt("Usage: %s [DATASET...]\n"
                "    [--help]               Print help and exit\n"
                "    [--version]            Print version and exit\n"
                "    [--config=<file>]      Set the configuration file to use\n"
                "    [--cache=<dir>]        Set the cache directory to use\n"
                "    [-s|--state=<file>]    Load the given state.\n"
                "    [--tracking=<t>]       Set tracking: off, eqevent, or dtrack. Default is off.\n"
                "    [-E|--equalizer]       Start in Equalizer mode; do not start GUI.\n"
                "Report bugs to <%s>.",
                program_name, PACKAGE_BUGREPORT);
    }
    if (version.value() || help.value()) {
        return 0;
    }

#if HAVE_LIBEQUALIZER
    if (arguments.size() > 0 && arguments[0] == "--eq-client") {
        try {
            class state dummy_state;    // this client will get its state from the Eq server
            eq_context eqctx(&argc, argv, &dummy_state);
        }
        catch (std::exception &e) {
            msg::err("%s", e.what());
            return 1;
        }
        return 0;
    }
#endif

    uuid app_uuid;
    app_uuid.generate();
    std::string app_id = app_uuid.to_string();
    std::string conf_file = config.value().empty() ? fio::homedir() + "/.config/" + PACKAGE_TARNAME + "/" + PACKAGE_TARNAME + ".conf" : config.value();
    std::string cache_dir = cache.value().empty() ? fio::homedir() + "/.cache/" + PACKAGE_TARNAME : cache.value();
    int track = (tracking.value().compare("off") == 0 ? 0
            : tracking.value().compare("eqevent") == 0 ? 1
            : 2);

    int retval = 0;
    try {
        fio::mkdir_p(fio::dirname(conf_file));
        fio::mkdir_p(cache_dir);
        class state master_state(msg::level(), app_id, conf_file, cache_dir, track);
#if W32
        msg::set_file(fio::open(fio::dirname(conf_file) + "/log.txt", "w"));
#endif
        if (!state.value().empty()) {
            master_state.load_statefile(state.value());
            msg::dbg("Loaded state file %s", state.value().c_str());
        }
        for (size_t i = 0; i < arguments.size(); i++) {
            master_state.open_database(arguments[i]);
        }
        if (state.value().empty() && arguments.size() > 0) {
            master_state.reset_viewer();
        }

        if (equalizer.value()) {
#if HAVE_LIBEQUALIZER
            eq_context eqctx(&argc, argv, &master_state);
            while (eqctx.is_running()) {
                eqctx.render();
            }
#else
            throw exc("This version of " PACKAGE_NAME " was compiled without support for Equalizer.");
#endif
        } else {
            retval = gui_run(&master_state);
        }
    }
    catch (std::exception& e) {
        msg::err("%s", e.what());
        retval = 1;
    }
    if (!equalizer.value()) {
        gui_deinitialize();
    }
    return retval;
}
