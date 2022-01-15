/*  python bindings for cssed's plugable interface (c) Iago Rubio 2004-2005
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <Python.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gmodule.h>
#include <plugin.h>

//#include <unistd.h>

#include "pythonscript.h"
#include "cssed-pyobject.h"
#include "pycssed-constants-table.h"

#ifdef WIN32
# define index strchr
#endif

void
pythonscript_plugin_create_menu (CssedPlugin *plugin);

/*
	You can use the gmodule provided functions:
	const gchar* g_module_check_init (GModule *module);
	void g_module_unload  (GModule *module);
	
	for preinitialization and post unload steps
	respectively.
*/

// module entry point. This must return a pointer to the plugin. Imported by
// cssed to load the plugin pointer.
G_MODULE_EXPORT CssedPlugin* init_plugin(void);
// cssed plugin creation function member of the plugin structure.
// Will be called by cssed through the plugin structure.
gboolean load_pythonscript_plugin ( CssedPlugin* );
// cssed plugin cleaning function member of the plugin's structure to destroy 
// the UI. You must free all memory used here.
// Will be called by cssed through the plugin structure.
void clean_pythonscript_plugin ( CssedPlugin* );

// the plugin structure
static CssedPlugin pythonscript_plugin;

// this will return the plugin to the caller
G_MODULE_EXPORT CssedPlugin* init_plugin()
{
	PythonPluginData* data;

	data = g_malloc0(sizeof(PythonPluginData));

	pythonscript_plugin.name = _("Python script");
	pythonscript_plugin.description = _("Adds a python scripting interface to cssed");
	pythonscript_plugin.load_plugin = &load_pythonscript_plugin;
	pythonscript_plugin.clean_plugin = &clean_pythonscript_plugin;
	pythonscript_plugin.user_data = data;									

	return &pythonscript_plugin;
}

// hash table utils
void       
glist_foreach_free_callback_data (gpointer data,  gpointer user_data)
{
	PythonScriptCallbackData *script_data;

	script_data = (PythonScriptCallbackData*) data;
	
	g_free (script_data->scriptname);
	g_free (script_data);
}

void       
glist_foreach_widget_destroy (gpointer data,  gpointer user_data)
{
	gtk_widget_destroy (data);
}
// to destroy UI and stuff. called by cssed.
void 
clean_pythonscript_plugin ( CssedPlugin* p )
{
	PythonPluginData* data;

	data = p->user_data;
	gtk_widget_destroy(GTK_WIDGET(data->pythonmenu));
	gtk_widget_destroy(GTK_WIDGET(data->pythonpopmenu));	
	gtk_widget_destroy(GTK_WIDGET(data->separator_menuitem));
	g_list_foreach (data->allocated_items, glist_foreach_free_callback_data, NULL);
	pycssed_constans_destroy_table (p);
	g_free( data );
}

// cleans a bit the script names. It will take "the_script.py" returning "The script"
// It to be used on scripts' menu.
gchar*
pythonscript_plugin_makeup_script_name ( const gchar * name )
{
	gchar *newname, *iterator, *utf8;
	const gchar* lastconverted;
 
	// only will reach here file names with the *.py termination
	newname = g_strdup (name);
	newname[strlen(newname)-3] = '\0';
	iterator = newname;

	while ( (iterator = index(iterator, '_')) != NULL )
	{
		*iterator = ' ';
	}

	if ( g_utf8_validate(newname, -1, &lastconverted) ){
		newname[0] = g_unichar_toupper(newname[0]);
		return newname;
	}else{
		utf8 = g_locale_to_utf8  (newname, strlen(newname), NULL, NULL, NULL);
		g_free (newname);
        if ( utf8 ){
			utf8[0] = g_unichar_toupper(utf8[0]);
			return utf8;
		}else{
			g_warning("Cannot convert script name to utf8");
			return NULL;
		}
	}	
}

void        
pythonscript_plugin_script_menu_callback   (GtkMenuItem *menuitem,
                                            gpointer user_data)
{
	gchar *filename;
	PythonScriptCallbackData *data;

	data = (PythonScriptCallbackData*) user_data;
	filename = data->scriptname;

	if( g_file_test(filename, G_FILE_TEST_EXISTS) ){
		pycssed_run_file_by_callback_data (data);
	}else{
		cssed_plugin_error_message (_("Cannot execute script"), _("Script does not exists\n\n%s"), filename);
	}
}

// recursively read all files in the plugins directory and 
// build a couple of menus ( menu bar and pop menu ).
void
pythonscript_plugin_parse_dir ( CssedPlugin* plugin, gchar* dirname, GtkWidget* parent_menu, GtkWidget* parent_popmenu )
{
	GDir *dir;
	GError *error = NULL;
	GPatternSpec *pattern;
	G_CONST_RETURN gchar *filename;
	gchar *fullname, *newname;
	GtkWidget* menuitem, *popmenuitem, *menu, *popmenu;
	PythonPluginData* data;
	PythonScriptCallbackData *cdata;

	data = plugin->user_data;

	pattern = g_pattern_spec_new ("*.py");	
	dir = g_dir_open (dirname, 0, &error);

	if (dir != NULL)
	{
		filename = g_dir_read_name (dir);
		while (filename)
		{
			fullname = g_strconcat (dirname, G_DIR_SEPARATOR_S, filename, NULL);
			if( g_file_test (fullname, G_FILE_TEST_IS_DIR) )
			{	
				menuitem = gtk_menu_item_new_with_label (filename);
				gtk_widget_show (menuitem);
				gtk_container_add (GTK_CONTAINER (parent_menu), menuitem);

				menu = gtk_menu_new ();
				gtk_widget_show (menu);
				gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem), menu);
				
				popmenuitem = gtk_menu_item_new_with_label (filename);
				gtk_widget_show (popmenuitem);
				gtk_container_add (GTK_CONTAINER (parent_popmenu), popmenuitem);	
				
				popmenu = gtk_menu_new ();
				gtk_widget_show (popmenu);
				gtk_menu_item_set_submenu (GTK_MENU_ITEM (popmenuitem), popmenu);

				pythonscript_plugin_parse_dir ( plugin, fullname, menu,  popmenu );
				g_free (fullname);
			}
#ifndef WIN32
			else if ( g_pattern_match_string (pattern, filename) && g_file_test (fullname, G_FILE_TEST_IS_EXECUTABLE) )
#else
			else if ( g_pattern_match_string (pattern, filename) )
#endif
			{		
				
				cdata = g_malloc0(sizeof(PythonScriptCallbackData));
				cdata->plugin = plugin;
				cdata->scriptname = fullname;

				newname = pythonscript_plugin_makeup_script_name (filename);
				if ( newname ){
					menuitem = gtk_menu_item_new_with_label (newname);
					popmenuitem = gtk_menu_item_new_with_label (newname);
					g_free (newname);
				}else{
					menuitem = gtk_menu_item_new_with_label (filename);
					popmenuitem = gtk_menu_item_new_with_label (filename);
				}
				gtk_widget_show (menuitem);
				gtk_widget_show (popmenuitem);
				
				gtk_container_add (GTK_CONTAINER (parent_menu), menuitem);
				gtk_container_add (GTK_CONTAINER (parent_popmenu), popmenuitem);
				
				g_signal_connect (G_OBJECT(menuitem), "activate", 
								  G_CALLBACK(pythonscript_plugin_script_menu_callback),
								  (gpointer) cdata);
				g_signal_connect (G_OBJECT(popmenuitem), "activate", 
								  G_CALLBACK(pythonscript_plugin_script_menu_callback),
								  (gpointer) cdata);
								  								  
				data->allocated_items = g_list_append (data->allocated_items, cdata);
				
				
			}
			else
			{
				g_free (fullname);
			}
			filename = g_dir_read_name (dir);
		}
		g_dir_close (dir);
	}
	else
	{
		// FIXME warn user
		g_error_free(error);
	}
	g_pattern_spec_free (pattern);
}

void
pythonscript_plugin_fill_scripts ( CssedPlugin* plugin )
{
	gchar* scripts_dir;
	PythonPluginData* data;
#ifdef WIN32
	gchar *globaldir;
#endif

	data = (PythonPluginData*) plugin->user_data;

	// user scripts
	scripts_dir = g_strdup_printf ("%s%s", g_get_home_dir(), USER_SCRIPTS_DIR);
	if( g_file_test(scripts_dir, G_FILE_TEST_IS_DIR) ){
		if( !GTK_WIDGET_VISIBLE(data->user_pythonscripts_menu_item) ) gtk_widget_show (data->user_pythonscripts_menu_item);
		if( !GTK_WIDGET_VISIBLE(data->user_pythonscripts_menu_item) ) gtk_widget_show (data->user_pythonscripts_popmenu_item);
		pythonscript_plugin_parse_dir ( plugin, scripts_dir, data->user_pythonscripts_menu,  data->user_pythonscripts_popmenu);
	}else{
		gtk_widget_hide( GTK_WIDGET(data->user_pythonscripts_menu_item) );
		gtk_widget_hide( GTK_WIDGET(data->user_pythonscripts_popmenu_item) );
	}
	g_free (scripts_dir);

	// system scripts
#ifdef WIN32
	globaldir =  g_win32_get_package_installation_directory
                                            (CSSED_REGISTRY_NAME,
                                             NULL);
	scripts_dir = g_strdup_printf ("%s%s", globaldir, GLOBAL_SCRIPTS_DIR);
	g_free(globaldir);
#else
	scripts_dir = g_strdup_printf ("%s%s", PACKAGE_LIB_DIR, GLOBAL_SCRIPTS_DIR);
#endif
	if( g_file_test(scripts_dir, G_FILE_TEST_IS_DIR) ){
		if( !GTK_WIDGET_VISIBLE(data->system_pythonscripts_menu_item) ) gtk_widget_show (data->system_pythonscripts_menu_item);
		if( !GTK_WIDGET_VISIBLE(data->system_pythonscripts_menu_item) ) gtk_widget_show (data->system_pythonscripts_popmenu_item);		
		pythonscript_plugin_parse_dir ( plugin, scripts_dir, data->system_pythonscripts_menu, data->system_pythonscripts_popmenu  );
	}else{
		gtk_widget_hide( GTK_WIDGET(data->system_pythonscripts_menu_item) );
		gtk_widget_hide( GTK_WIDGET(data->system_pythonscripts_popmenu_item) );		
	}
	g_free (scripts_dir);	
}

gboolean
load_pythonscript_plugin ( CssedPlugin* plugin )
{
	PythonPluginData* data;
	GtkWidget* cssed_popmenu;

	cssed_popmenu = cssed_plugin_get_pop_menu( plugin );
		
	//pycssed_initialize (plugin); 
	data = (PythonPluginData*) plugin->user_data;
	pythonscript_plugin_create_menu (plugin);
	gtk_widget_show (data->pythonmenu);
	gtk_widget_show (data->pythonpopmenu);
	gtk_widget_show (data->separator_menuitem);	
	
	cssed_add_menu_item_to_plugins_menu (plugin, data->pythonmenu);
	
	gtk_container_add (GTK_CONTAINER (cssed_popmenu), data->separator_menuitem);
	gtk_container_add (GTK_CONTAINER (cssed_popmenu), data->pythonpopmenu);
	
		
	pythonscript_plugin_fill_scripts (plugin);
	pycssed_constans_initialize_table (plugin);

	return TRUE;
}

void        
pythonscript_plugin_runscript_menu_callback   (GtkMenuItem *menuitem,
                                            gpointer user_data)
{
	gchar *filename;
	CssedPlugin *plugin;
	PythonScriptCallbackData *cdata;

	plugin = (CssedPlugin*) user_data;
	
	filename = cssed_plugin_prompt_for_file_to_open(plugin, _("Select python script to execute"), NULL);
	if( filename )
	{
		cdata = g_malloc0(sizeof(PythonScriptCallbackData));
		cdata->plugin = plugin;
		cdata->scriptname = filename;

		pycssed_run_file_by_callback_data (cdata);

		g_free (filename);
		g_free (cdata);
	}
}

void        
pythonscript_plugin_runcurrent_menu_callback   (GtkMenuItem *menuitem,
                                            gpointer user_data)
{
	gchar *buffer;
	CssedPlugin *plugin;

	plugin = (CssedPlugin*) user_data;
	
	buffer = cssed_plugin_get_text(plugin);
	if( buffer )
	{
		pycssed_run_buffer (plugin, buffer);
		g_free (buffer);
	}
}

void        
pythonscript_plugin_refresh_menu_callback   (GtkMenuItem *menuitem,
                                            gpointer user_data)
{
	CssedPlugin *plugin;
	PythonPluginData* data;
	GList* list;

	plugin = (CssedPlugin*) user_data;
	data = (PythonPluginData*) plugin->user_data;

	list = GTK_MENU_SHELL(data->system_pythonscripts_menu)->children;
	g_list_foreach (list, glist_foreach_widget_destroy, NULL);

	list = GTK_MENU_SHELL(data->user_pythonscripts_menu)->children;
	g_list_foreach (list, glist_foreach_widget_destroy, NULL);

	list = GTK_MENU_SHELL(data->system_pythonscripts_popmenu)->children;
	g_list_foreach (list, glist_foreach_widget_destroy, NULL);

	list = GTK_MENU_SHELL(data->user_pythonscripts_popmenu)->children;
	g_list_foreach (list, glist_foreach_widget_destroy, NULL);
	
	g_list_foreach (data->allocated_items, glist_foreach_free_callback_data, NULL); // clean old allocated strings
	data->allocated_items = NULL;

	pythonscript_plugin_fill_scripts (plugin); // parse scripts directory
}

void
pythonscript_plugin_create_menu (CssedPlugin *plugin)
{
	GtkWidget *pythonmenu;
	GtkWidget *pythonpopmenu;
	GtkWidget *separator_menuitem;	
	GtkWidget *python_scripts_menu;
	GtkWidget *python_scripts_popmenu;
	GtkWidget *runfilemenu;
	GtkWidget *runcurrent;
	GtkWidget *image;
	GtkWidget *separator;
	GtkWidget *system_pythonscripts_menu_item;
	GtkWidget *system_pythonscripts_popmenu_item;
	
	GtkWidget *system_pythonscripts_menu;
	GtkWidget *system_pythonscripts_popmenu;
	GtkWidget *user_pythonscripts_menu_item;
	GtkWidget *user_pythonscripts_popmenu_item;
	GtkWidget *user_pythonscripts_menu;
	GtkWidget *user_pythonscripts_popmenu;
	GtkWidget *refresh;
	PythonPluginData* data;

	data = (PythonPluginData*) plugin->user_data;
 
	pythonmenu = gtk_menu_item_new_with_mnemonic (_("Python scripts"));
	gtk_widget_show (pythonmenu);
 
	pythonpopmenu = gtk_menu_item_new_with_mnemonic (_("Python scripts"));
	gtk_widget_show (pythonpopmenu);
		
	separator_menuitem = gtk_menu_item_new();
	gtk_widget_set_sensitive( separator_menuitem, FALSE );
			
	python_scripts_menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (pythonmenu), python_scripts_menu);	
		
	python_scripts_popmenu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (pythonpopmenu), python_scripts_popmenu);
	
	user_pythonscripts_menu_item = gtk_menu_item_new_with_mnemonic (_("User scripts"));
	gtk_widget_show (user_pythonscripts_menu_item);
	gtk_container_add (GTK_CONTAINER (python_scripts_menu), user_pythonscripts_menu_item);
		
	user_pythonscripts_popmenu_item = gtk_menu_item_new_with_mnemonic (_("User scripts"));
	gtk_widget_show (user_pythonscripts_popmenu_item);
	gtk_container_add (GTK_CONTAINER (python_scripts_popmenu), user_pythonscripts_popmenu_item);
	
	// user's scripts menu
	user_pythonscripts_menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (user_pythonscripts_menu_item), user_pythonscripts_menu);
	
	// user's scripts popmenu
	user_pythonscripts_popmenu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (user_pythonscripts_popmenu_item), user_pythonscripts_popmenu);
	
	system_pythonscripts_menu_item = gtk_menu_item_new_with_mnemonic (_("System scripts"));
	gtk_widget_show (system_pythonscripts_menu_item);
	gtk_container_add (GTK_CONTAINER (python_scripts_menu), system_pythonscripts_menu_item);	
	
	system_pythonscripts_popmenu_item = gtk_menu_item_new_with_mnemonic (_("System scripts"));
	gtk_widget_show (system_pythonscripts_popmenu_item);
	gtk_container_add (GTK_CONTAINER (python_scripts_popmenu), system_pythonscripts_popmenu_item);
	
	// global scripts menu
	system_pythonscripts_menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (system_pythonscripts_menu_item), system_pythonscripts_menu);
	
	// global scripts popmenu
	system_pythonscripts_popmenu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (system_pythonscripts_popmenu_item), system_pythonscripts_popmenu);
	
	runfilemenu = gtk_image_menu_item_new_with_mnemonic (_("Run file"));
	gtk_widget_show (runfilemenu);
	gtk_container_add (GTK_CONTAINER (python_scripts_menu), runfilemenu);
	
	image = gtk_image_new_from_stock ("gtk-execute", GTK_ICON_SIZE_MENU);
	gtk_widget_show (image);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (runfilemenu), image);
	
	runcurrent = gtk_image_menu_item_new_with_mnemonic (_("Run current document"));
	gtk_widget_show (runcurrent);
	gtk_container_add (GTK_CONTAINER (python_scripts_menu), runcurrent);
	
	image = gtk_image_new_from_stock ("gtk-execute", GTK_ICON_SIZE_MENU);
	gtk_widget_show (image);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (runcurrent), image);

	separator = gtk_menu_item_new ();
	gtk_widget_show (separator);
	gtk_container_add (GTK_CONTAINER (python_scripts_menu), separator);
	gtk_widget_set_sensitive (separator, FALSE);

	refresh = gtk_image_menu_item_new_with_mnemonic (_("Re-read script directories"));
	gtk_widget_show (refresh);
	gtk_container_add (GTK_CONTAINER (python_scripts_menu), refresh);
	
	image = gtk_image_new_from_stock ("gtk-refresh", GTK_ICON_SIZE_MENU);
	gtk_widget_show (image);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (refresh), image);

	data->pythonmenu = pythonmenu; // topmost menu
	data->system_pythonscripts_menu = system_pythonscripts_menu;
	data->system_pythonscripts_menu_item = system_pythonscripts_menu_item;
	data->user_pythonscripts_menu = user_pythonscripts_menu;
	data->user_pythonscripts_menu_item = user_pythonscripts_menu_item; 
	
	data->pythonpopmenu = pythonpopmenu; // topmost menu
	data->system_pythonscripts_popmenu = system_pythonscripts_popmenu;
	data->system_pythonscripts_popmenu_item = system_pythonscripts_popmenu_item;
	data->user_pythonscripts_popmenu = user_pythonscripts_popmenu;
	data->user_pythonscripts_popmenu_item = user_pythonscripts_popmenu_item; 
	
	data->separator_menuitem = separator_menuitem; // topmost menu	
	
		
	data->allocated_items = NULL; // items that should be freed on destroy

	g_signal_connect (G_OBJECT(runfilemenu), "activate", 
					  G_CALLBACK(pythonscript_plugin_runscript_menu_callback),
					  (gpointer) plugin);

	g_signal_connect (G_OBJECT(runcurrent), "activate", 
					  G_CALLBACK(pythonscript_plugin_runcurrent_menu_callback),
					  (gpointer) plugin);
					  
	g_signal_connect (G_OBJECT(refresh), "activate", 
					  G_CALLBACK(pythonscript_plugin_refresh_menu_callback),
					  (gpointer) plugin);
}
