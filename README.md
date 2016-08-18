Find trailing whitespace Plugin for Geany
=========================

About
-----------

Find trailing whitespace is a plugin for Geany that finds and selects trailing whitespace.


Building and Installing
-----------------------

Download the plugin from https://github.com/leifmariposa/geany-find-trailing-whitespace-plugin

Then run the following commands:

```bash
$ make
$ sudo make install
```

Using the Plugin
----------------

After having enabled the plugin inside Geany through Geany's plugin manager,
you'll need to setup a keybinding. Go to the preferences, and under the Keybindings tab 
set the Find trailing whitespace keybinding, e. g. <Primary><Shift>t.

Using the plugin is simple. Press the keybinding that you selected and the plugin will search for trailing 
whitespace, with start from cursors current position. If any trailing whitespace is found it will be selected. 
Press the keybinding again to continue to search.

License
----------------

This plugin is distributed under the terms of the GNU General Public License
as published by the Free Software Foundation, either version 2 of the
License, or (at your option) any later version. You should have received a copy
of the GNU General Public License along with this plugin.  If not, see
<http://www.gnu.org/licenses/>.

Contact
----------------

You can email me at &lt;leifmariposa(at)hotmail(dot)com&gt;


Bug reports and feature requests
----------------

To report a bug or ask for a new feature, please use the tracker
on GitHub: https://github.com/leifmariposa/geany-find-trailing-whitespace-plugin/issues
