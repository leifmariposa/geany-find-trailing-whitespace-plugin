/*
 *
 *  Copyright (C) 2016  Leif Persson <leifmariposa@hotmail.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <geanyplugin.h>

#define D(x) /*x*/

#define SSM(s, m, w, l) scintilla_send_message(s, m, w, l)

/**********************************************************************/
static const char *PLUGIN_NAME = "Find trailing whitespace";
static const char *PLUGIN_DESCRIPTION = "Finds trailing whitespace";
static const char *PLUGIN_VERSION = "0.1";
static const char *PLUGIN_AUTHOR = "Leif Persson <leifmariposa@hotmail.com>";
static const char *PLUGIN_KEY_NAME = "find_trailing_whitespace";



/**********************************************************************/
GeanyPlugin *geany_plugin;


/**********************************************************************/
enum
{
	KB_GOTO_FUNCTION,
	KB_COUNT
};


/**********************************************************************/
D(static void log_debug(const gchar* s, ...)
{
	gchar* format = g_strconcat("[CTR DEBUG] : ", s, "\n", NULL);
	va_list l;
	va_start(l, s);
	g_vprintf(format, l);
	g_free(format);
	va_end(l);
})


/**********************************************************************/
gboolean editor_line_in_view(GeanyEditor *editor, gint line)
{
	gint vis1, lines_on_screen;

	g_return_val_if_fail(editor != NULL, FALSE);

	/* If line is wrapped the result may occur on another virtual line than the first and may be
	 * still hidden, so increase the line number to check for the next document line */
	if (SSM(editor->sci, SCI_WRAPCOUNT, line, 0) > 1)
		line++;

	line = SSM(editor->sci, SCI_VISIBLEFROMDOCLINE, line, 0);	/* convert to visible line number */
	vis1 = SSM(editor->sci, SCI_GETFIRSTVISIBLELINE, 0, 0);
	lines_on_screen = SSM(editor->sci, SCI_LINESONSCREEN, 0, 0);

	return (line >= vis1 && line < vis1 + lines_on_screen);
}


/**********************************************************************/
void editor_scroll_to_line(GeanyEditor *editor, gint line, gfloat percent_of_view)
{
	gint lines_on_screen;
	GtkWidget *wid;

	g_return_if_fail(editor != NULL);

	wid = GTK_WIDGET(editor->sci);

	if (! gtk_widget_get_window(wid) || ! gdk_window_is_viewable(gtk_widget_get_window(wid)))
		return;	/* prevent gdk_window_scroll warning */

	if (line == -1)
		line = sci_get_current_line(editor->sci);

	/* sci 'visible line' != doc line number because of folding and line wrapping */
	/* calling SCI_VISIBLEFROMDOCLINE for line is more accurate than calling
	 * SCI_DOCLINEFROMVISIBLE for vis1. */
	line = SSM(editor->sci, SCI_VISIBLEFROMDOCLINE, line, 0);
	lines_on_screen = SSM(editor->sci, SCI_LINESONSCREEN, 0, 0);
	line = line - lines_on_screen * percent_of_view;
	SSM(editor->sci, SCI_SETFIRSTVISIBLELINE, line, 0);
	sci_scroll_caret(editor->sci); /* needed for horizontal scrolling */
}


/**********************************************************************/
static void find_trailing_whitespace()
{
	D(log_debug("%s:%s", __FILE__, __FUNCTION__));

	GeanyDocument      *doc = document_get_current();
	GeanyEditor        *editor = doc->editor;
	ScintillaObject    *sci = editor->sci;
	struct              Sci_TextToFind ttf;
	gint                match_flag = SCFIND_REGEXP;
	const gchar *text = "[ \t]+$";
	gint                start_pos;
	gint                search_pos;

	start_pos = sci_get_selection_end(editor->sci);

	ttf.chrg.cpMin = start_pos;
	ttf.chrg.cpMax = sci_get_length(sci);
	ttf.lpstrText  = text;

	search_pos = sci_find_text(sci, match_flag, &ttf);

	/* if no match, search start (or end) to cursor */
	if (search_pos == -1)
	{
		ttf.chrg.cpMin = 0;
		ttf.chrg.cpMax =  start_pos + strlen(text);
		search_pos = sci_find_text(sci, match_flag, &ttf);
	}

	if (search_pos != -1)
	{
		gint line = sci_get_line_from_position(sci, ttf.chrgText.cpMin);

		/* unfold maybe folded results */
		sci_ensure_line_is_visible(sci, line);

		sci_set_selection_start(sci, ttf.chrgText.cpMin);
		sci_set_selection_end(sci, ttf.chrgText.cpMax);

		if (!editor_line_in_view(doc->editor, line))
		{	/* we need to force scrolling in case the cursor is outside of the current visible area
			 * GeanyDocument::scroll_percent doesn't work because sci isn't always updated
			 * while searching */
			editor_scroll_to_line(editor, -1, 0.3F);
		} else {
			sci_scroll_caret(doc->editor->sci); /* may need horizontal scrolling */
		}

		ui_set_statusbar(TRUE, _("Trailing whitespace found at line %d"), line + 1);
	}
	else
	{
		ui_set_statusbar(TRUE, _("No trailing whitespace was found"));
		gdk_beep();
	}
}


/**********************************************************************/
static void item_activate_cb(G_GNUC_UNUSED GtkMenuItem *menuitem, G_GNUC_UNUSED gpointer user_data)
{
	D(log_debug("%s:%s", __FILE__, __FUNCTION__));

	find_trailing_whitespace();
}


/**********************************************************************/
static void kb_activate(G_GNUC_UNUSED guint key_id)
{
	D(log_debug("%s:%s", __FILE__, __FUNCTION__));

	find_trailing_whitespace();
}


/**********************************************************************/
static gboolean init(GeanyPlugin *plugin, G_GNUC_UNUSED gpointer pdata)
{
	D(log_debug("%s:%s", __FILE__, __FUNCTION__));

	GtkWidget* edit_menu = ui_lookup_widget(plugin->geany_data->main_widgets->window, "search1_menu");

	GtkWidget *main_menu_item;
	main_menu_item = gtk_menu_item_new_with_mnemonic(PLUGIN_NAME);
	gtk_widget_show(main_menu_item);
	gtk_container_add(GTK_CONTAINER(edit_menu), main_menu_item);

	GeanyKeyGroup *key_group = plugin_set_key_group(plugin, PLUGIN_KEY_NAME, KB_COUNT, NULL);
	keybindings_set_item(key_group, KB_GOTO_FUNCTION, kb_activate, 0, 0, PLUGIN_KEY_NAME, PLUGIN_NAME, main_menu_item);

	g_signal_connect(main_menu_item, "activate", G_CALLBACK(item_activate_cb), NULL);
	geany_plugin_set_data(plugin, main_menu_item, NULL);

	return TRUE;
}


/**********************************************************************/
static void cleanup(G_GNUC_UNUSED GeanyPlugin *plugin, gpointer pdata)
{
	D(log_debug("%s:%s", __FILE__, __FUNCTION__));

	GtkWidget *main_menu_item = (GtkWidget*)pdata;
	gtk_widget_destroy(main_menu_item);
}


/**********************************************************************/
G_MODULE_EXPORT
void geany_load_module(GeanyPlugin *plugin)
{
	D(log_debug("%s:%s", __FILE__, __FUNCTION__));

	geany_plugin = plugin;
	plugin->info->name = PLUGIN_NAME;
	plugin->info->description = PLUGIN_DESCRIPTION;
	plugin->info->version = PLUGIN_VERSION;
	plugin->info->author = PLUGIN_AUTHOR;
	plugin->funcs->init = init;
	plugin->funcs->cleanup = cleanup;
	GEANY_PLUGIN_REGISTER(plugin, 225);
}
