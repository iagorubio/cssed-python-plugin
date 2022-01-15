/*  cssed python binding for the plugable interface (c) Iago Rubio 2004-2005
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*
 * Standard gettext macros.
 */
#ifdef ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String) dgettext (PACKAGE, String)
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define N_(String) (String)
#  endif
#else
#  define textdomain(String) (String)
#  define gettext(String) (String)
#  define dgettext(Domain,Message) (Message)
#  define dcgettext(Domain,Message,Type) (Message)
#  define bindtextdomain(Domain,Directory) (Domain)
#  define _(String) (String)
#  define N_(String) (String)
#endif

#ifdef WIN32
# define USER_SCRIPTS_DIR "\\.cssed\\pythonscripts"
# define USER_INIT_SCRIPT "\\__pycssedinit__.py"
#else
# define USER_SCRIPTS_DIR "/.cssed/pythonscripts"
# define USER_INIT_SCRIPT "/__pycssedinit__.py"
#endif

#define GLOBAL_SCRIPTS_DIR "python"


typedef struct _PythonScriptCallbackData {
	CssedPlugin *plugin;
	gchar *scriptname;
} PythonScriptCallbackData;

// data used by this module
typedef struct _PythonPluginData {
	GtkWidget* pythonmenu; // topmost menu
	GtkWidget* pythonpopmenu; // 
	GtkWidget* separator_menuitem; // 
	GtkWidget* system_pythonscripts_menu; // system stored python scripts menu
	GtkWidget* system_pythonscripts_menu_item;
	GtkWidget* user_pythonscripts_menu; // user stored python scripts menu
	GtkWidget* user_pythonscripts_menu_item;	
	GtkWidget* system_pythonscripts_popmenu; // system stored python scripts menu
	GtkWidget* system_pythonscripts_popmenu_item;
	GtkWidget* user_pythonscripts_popmenu; // user stored python scripts menu
	GtkWidget* user_pythonscripts_popmenu_item;
	GList* allocated_items; // items that should be freed on destroy
	GHashTable* constants_table;
} PythonPluginData;

