#include <string.h>
#include "attachmentpoint.h"
#include "filehtal.h"
#include "mainwindow.h"
#include "pluginserver.h"
#include "virtualnode.h"

AttachmentPoint::AttachmentPoint(AttachmentPoint *that)
{
	reset_parameters();
	copy_from(*that);
}

AttachmentPoint::AttachmentPoint(MainWindow *mwindow)
{
	reset_parameters();
	this->mwindow = mwindow;
}

AttachmentPoint::~AttachmentPoint()
{
	if(plugin_server)
	{
		detach();
	}
	reset_parameters();
}

AttachmentPoint& AttachmentPoint::operator=(const AttachmentPoint &that)
{
	copy_from(that);
	return *this;
}

int AttachmentPoint::operator==(const AttachmentPoint &that)
{
	int result = 0;

	if(identical(that)) result = 1;

	return result;
return 0;
}

int AttachmentPoint::identical(const AttachmentPoint &that)
{
	int result = 0;

	if(plugin_type == that.plugin_type)
	{
		switch(plugin_type)
		{
			case 0:
				result = 1;
				break;
			
			case 1:
				if(!strcmp(plugin_title, that.plugin_title) &&
					!strcmp(plugin_data, that.plugin_data)) result = 1;
				break;
			
			case 2:
				if(shared_plugin_location == that.shared_plugin_location) result = 1;
				break;

			case 3:
				if(shared_module_location == that.shared_module_location) result = 1;
				break;
		}
	}

	return result;
return 0;
}

int AttachmentPoint::copy_from(const AttachmentPoint &that)
{
	detach();
	shared_plugin_location = that.shared_plugin_location;
	shared_module_location = that.shared_module_location;
	in = that.in;
	out = that.out;
	show = that.show;
	on = that.on;
	plugin_type = that.plugin_type;
	mwindow = that.mwindow;
	strcpy(plugin_title, that.plugin_title);
	strcpy(plugin_data, that.plugin_data);

// Need to close plugin if it's open before replacing.
	if(that.plugin_server) plugin_server = new PluginServer(*(that.plugin_server));
return 0;
}

int AttachmentPoint::dump()
{
	printf("		attachmentpoint %x\n", this);
	if(plugin_server) plugin_server->dump();
return 0;
}

int AttachmentPoint::reset_parameters()
{
	in = out = show = on = 0;
	plugin_type = 0;
	plugin_server = 0;
	plugin_type = 0;
	plugin_data[0] = 0;
	strcpy(plugin_title, default_title());
	new_total_input_buffers = 0;
	total_input_buffers = 0;
return 0;
}

int AttachmentPoint::attach_virtual_plugin(VirtualNode *virtual_plugin)
{
	int in_buffer_number = 0;

// add virtual plugin to list
	new_virtual_plugins.append(virtual_plugin);

// start plugin and notify of buffers
	if(plugin_server && on)
	{
		if(plugin_server->multichannel)
			in_buffer_number = new_total_input_buffers++;
		else
		{
			in_buffer_number = 0;
			new_total_input_buffers = 1;
		}
	}

	return in_buffer_number;
return 0;
}

int AttachmentPoint::sort(VirtualNode *virtual_plugin)
{
	int result = 0;

	for(int i = 0; i < new_virtual_plugins.total && !result; i++)
	{
// if a virtual plugin attached to this isn't waiting for this return 1
		if(!new_virtual_plugins.values[i]->waiting_real_plugin) result = 1;
	}

	return result;
return 0;
}

int AttachmentPoint::render_init(int realtime_sched, int duplicate)
{
	if(plugin_server && on)
	{
		if(new_virtual_plugins.total)
		{
// New plugins have started or continue.
			PluginServer* virtual_plugin_server;
			VirtualNode* virtual_node;
			int i;
			int need_new_plugins = (!virtual_plugins.total || virtual_plugins.total != new_virtual_plugins.total);

// Detach the old buffers.
			if(virtual_plugins.total)
			{
				if(plugin_server->multichannel) 
					virtual_plugins.values[0]->plugin_server->detach_buffers();
				else
				for(i = 0; i < virtual_plugins.total; i++)
				{
					virtual_plugins.values[i]->plugin_server->detach_buffers();
				}
			}

// Try to copy old plugin_servers to new plugin_servers.
			if(!need_new_plugins)
			{
				for(i = 0; i < virtual_plugins.total; i++)
				{
					new_virtual_plugins.values[i]->plugin_server = virtual_plugins.values[i]->plugin_server;
				}
			}

// Initialize the plugins
			for(i = 0; i < new_virtual_plugins.total; i++)
			{
				virtual_node = new_virtual_plugins.values[i];

// Start plugin servers if necessary
				if(need_new_plugins)
				{
					if(plugin_server->multichannel && new_virtual_plugins.values[0]->plugin_server)
					{
// Multichannel plugin and first plugin initialized.
						virtual_node->plugin_server = new_virtual_plugins.values[0]->plugin_server;
						virtual_plugin_server = virtual_node->plugin_server;
					}
					else
					{
// Multichannel plugin with no first plugin or single channel plugin.
						virtual_node->plugin_server = new PluginServer(*plugin_server);
						virtual_plugin_server = virtual_node->plugin_server;
						virtual_plugin_server->open_plugin();
					}
				}
				else
				{
					virtual_plugin_server = virtual_node->plugin_server;
				}

// Attach new buffers to the plugin.
				virtual_plugin_server->attach_input_buffer(virtual_node->input, 
						virtual_node->input_is_master ? virtual_node->double_buffers : 1, 
						virtual_node->input_is_master ? virtual_node->buffer_size : virtual_node->fragment_size, 
						virtual_node->fragment_size);
				virtual_plugin_server->attach_output_buffer(virtual_node->output, 
						virtual_node->output_is_master ? virtual_node->double_buffers : 1, 
						virtual_node->output_is_master ? virtual_node->buffer_size : virtual_node->fragment_size, 
						virtual_node->fragment_size);
			}

// Propogate configuration from the GUI plugin to every plugin_server.
			if(need_new_plugins)
			{
// Start up and pass configuration to all new plugins
				for(i = 0; i < (plugin_server->multichannel ? 1 : new_virtual_plugins.total); i++)
				{
					virtual_plugin_server = new_virtual_plugins.values[i]->plugin_server;
					virtual_plugin_server->init_realtime(realtime_sched);
// Send data to plugin
					virtual_plugin_server->gui_on = plugin_server->gui_on;
					virtual_plugin_server->send_gui_status(plugin_server->gui_on);
					virtual_plugin_server->get_configuration_change(plugin_data);
				}
			}
			else
			{
// Send new buffers to old plugins
				for(i = 0; i < (plugin_server->multichannel ? 1 : new_virtual_plugins.total); i++)
				{
					virtual_plugin_server = new_virtual_plugins.values[i]->plugin_server;
					virtual_plugin_server->restart_realtime();
				}
			}

// Delete the old plugins and swap in the new plugins
			if(need_new_plugins)
			{
// Delete the old plugin servers.
				render_stop(0);
			}
			else
			{
// Just delete the pointers to the old plugin servers.
				virtual_plugins.remove_all();
			}

// Swap in the new plugin pointers
			for(i = 0; i < new_virtual_plugins.total; i++)
			{
				virtual_plugins.append(new_virtual_plugins.values[i]);
			}
			new_virtual_plugins.remove_all();
			total_input_buffers = new_total_input_buffers;
		}
		else
		if(virtual_plugins.total)
		{
// Old plugins are now stopped
			render_stop(0);
		}
	}
	new_total_input_buffers = 0;
return 0;
}

int AttachmentPoint::render_stop(int duplicate)
{
// stop plugins
// Can't use the on value here because it may have changed.
	if(plugin_server && virtual_plugins.total && !duplicate)
	{
// close the plugins if not shared
		PluginServer *virtual_plugin_server;

		for(int i = 0; i < (plugin_server->multichannel ? 1 : virtual_plugins.total); i++)
		{
			virtual_plugin_server = virtual_plugins.values[i]->plugin_server;
			virtual_plugin_server->realtime_stop();
			virtual_plugin_server->close_plugin();
			delete virtual_plugin_server;
			virtual_plugins.values[i]->plugin_server = 0;
		}

// delete pointers to servers
		virtual_plugins.remove_all();
	}
return 0;
}

int AttachmentPoint::multichannel_shared(int search_new)
{
	if(search_new)
	{
		if(new_virtual_plugins.total && plugin_server && plugin_server->multichannel) return 1;
	}
	else
	{
		if(virtual_plugins.total && plugin_server && plugin_server->multichannel) return 1;
	}
	return 0;
return 0;
}

int AttachmentPoint::singlechannel()
{
	if(plugin_server && !plugin_server->multichannel) return 1;
	return 0;
return 0;
}


int AttachmentPoint::detach(int update_edits)
{
	if(plugin_type)
	{
		if(plugin_server)
		{
//			plugin_server->stop_gui();     // sends a completed command to the thread
			plugin_server->close_plugin();        // tell client thread to finish
			delete plugin_server;
			plugin_server = 0;
			plugin_data[0] = 0;
		}

		plugin_type = 0;
		strcpy(plugin_title, default_title());
// Sometimes detach is used right before attach
		if(update_edits) update_edit(0);
	}
return 0;
}

int AttachmentPoint::attach(int is_loading)
{
	int result = 0;

	if(plugin_type == 1)
	{
// this plugin needs to be forked
// find it in the plugindb using the plugin_title field
		ArrayList<PluginServer *> *db = mwindow->plugindb;
		PluginServer *original_server = 0;

		for(int i = 0; i < db->total && !result; i++)
		{
			if(!strcmp(db->values[i]->title, plugin_title))
			{
				original_server = db->values[i];
				result = 1;
			}
		}

		result = 0;
		if(original_server)
		{
// Set up plugin
			plugin_server = new PluginServer(*original_server);
			plugin_server->set_mainwindow(mwindow);

// Get configuration from plugin
			if(show)
			{
				show_gui();
				strcpy(plugin_data, plugin_server->save_data());
			}
			else
			if(!is_loading)
			{
// The configuration is loaded from the file when loading.
// Plugin must load defaults without GUI.
				plugin_server->open_plugin();
				plugin_server->load_defaults();
				strcpy(plugin_data, plugin_server->save_data());
				plugin_server->close_plugin();
			}
			result = 0;
		}
		else
		{
// no plugin by that name found
			plugin_server = 0;
			plugin_type = 0;
			strcpy(plugin_title, default_title());
			result = 1;
		}
	}
	update_edit(is_loading);
	return result;
return 0;
}

int AttachmentPoint::update(int plugin_type, int in, int out, const char* title, SharedPluginLocation *shared_plugin_location, SharedModuleLocation *shared_module_location)
{
	this->in = in; this->out = out;
	this->plugin_type = plugin_type;

	strcpy(this->plugin_title, title);

	if(shared_plugin_location && shared_module_location)
	{
		this->shared_plugin_location = *shared_plugin_location;
		this->shared_module_location = *shared_module_location;
	}
	update_derived();
return 0;
}

int AttachmentPoint::show_gui()
{
	if(plugin_server && !plugin_server->plugin_open)
	{
		char string[1024];
		sprintf(string, "%s: %s\n", get_module_title(), plugin_title);
		plugin_server->open_plugin(this, 0, string);
		if(plugin_data[0])
			plugin_server->get_configuration_change(plugin_data);
	}
return 0;
}

int AttachmentPoint::hide_gui()
{
	if(plugin_server && plugin_server->plugin_open)
	{
		strcpy(plugin_data, plugin_server->save_data());
		plugin_server->close_plugin();
	}
return 0;
}

int AttachmentPoint::set_show(int value)
{
	if(!plugin_server) return 0;

// Update GUI
	set_show_derived(value);
	show = value;
// Update DSP instances
	if(virtual_plugins.total)
	{
		for(int i = 0; i < (plugin_server->multichannel ? 1 : virtual_plugins.total); i++)
		{
			virtual_plugins.values[i]->plugin_server->send_gui_status(value);
		}
	}
	return 0;
return 0;
}


int AttachmentPoint::save(FileHTAL *htal, char *block_title)
{
	char string[1024];
	htal->tag.set_title(block_title);
	htal->tag.set_property("TYPE", (long)plugin_type);
	htal->append_tag();
	htal->append_newline();

// only save the title if there is a plugin
	if(in)
	{
		htal->tag.set_title("IN");
		htal->append_tag();
	}

	if(out)
	{
		htal->tag.set_title("OUT");
		htal->append_tag();
	}

	if(show)
	{
		htal->tag.set_title("SHOW");
		htal->append_tag();
	}

	if(on)
	{
		htal->tag.set_title("ON");
		htal->append_tag();
	}


// save extra information if it is a shared plugin
	switch(plugin_type)
	{
		case 2:
			shared_plugin_location.save(htal);
			break;
			
		case 3:
			shared_module_location.save(htal);
			break;
	}

	if(plugin_type)
	{
		if(plugin_server)
		{
			htal->tag.set_title("DATA");
			htal->append_tag();
			htal->append_newline();

			htal->append_text(plugin_data);

			htal->tag.set_title("/DATA");
			htal->append_tag();
			htal->append_newline();
		}

		htal->tag.set_title("TITLE");
		htal->append_tag();
		htal->append_text(plugin_title);
		htal->tag.set_title("/TITLE");
		htal->append_tag();
		htal->append_newline();
	}

	htal->append_newline();
	sprintf(string, "/%s", block_title);
	htal->tag.set_title(string);
	htal->append_tag();
	htal->append_newline();
return 0;
}

int AttachmentPoint::load(FileHTAL *htal, int track_offset, char *terminator)
{
	int result = 0;
	in = 0;
	out = 0;
	show = 0;
	on = 0;

	plugin_type = htal->tag.get_property("TYPE", (long)0);
	
	do{
		result = htal->read_tag();

		if(!result)
		{
			if(htal->tag.title_is(terminator))
			{
				result = 1;
			}
			else
			if(htal->tag.title_is("IN"))
			{
				in = 1;
			}
			else
			if(htal->tag.title_is("OUT"))
			{
				out = 1;
			}
			else
			if(htal->tag.title_is("SHOW"))
			{
				show = 1;
			}
			else
			if(htal->tag.title_is("ON"))
			{
				on = 1;
			}
			else
			if(htal->tag.title_is("SHARED"))
			{
				shared_plugin_location.load(htal, track_offset);
			}
			else
			if(htal->tag.title_is("MODULE"))
			{
				shared_module_location.load(htal, track_offset);
			}
			else
			if(htal->tag.title_is("DATA"))
			{
				htal->read_text_until("/DATA", plugin_data);
			}
			else
			if(htal->tag.title_is("TITLE"))
			{
				strcpy(plugin_title, htal->read_text());
			}
		}
	}while(!result);

	if(plugin_type == 1)
	{
// plugin must be an executable to start
		attach(1);
	}

	update_display();
return 0;
}

char* AttachmentPoint::get_module_title()
{
	return "AttachmentPoint::get_module_title";
}


int AttachmentPoint::get_configuration_change(char *data)
{
	strcpy(plugin_data, data);
	
	if(plugin_server->multichannel && virtual_plugins.total)
	{
		virtual_plugins.values[0]->plugin_server->get_configuration_change(plugin_data);
	}
	else
	for(int i = 0; i < virtual_plugins.total; i++)
	{
		virtual_plugins.values[i]->plugin_server->get_configuration_change(plugin_data);
	}
return 0;
}

int AttachmentPoint::render(int double_buffer_in, int double_buffer_out, 
				long fragment_position_in, long fragment_position_out,
				long size, int node_number, 
				long source_position, long source_len, 
				FloatAutos *autos,
				FloatAuto **start_auto,
				FloatAuto **end_auto,
				int reverse)
{
// This routine is called only for multichannel plugins.  The virtual node calls 
// the plugin server directly for other plugins.
	int result = 0;
	virtual_plugins.values[0]->plugin_server->arm_buffer(node_number,
							fragment_position_in,
							fragment_position_out,
							double_buffer_in,
							double_buffer_out);

	for(int i = 0; i < virtual_plugins.total; i++)
	{
// all virtual plugins must have render_count == 0 before rendering
		if(virtual_plugins.values[i]->render_count) result = 1;
	}

	if(!result)
	{
		if(virtual_plugins.values[0]->plugin_server)
		{
// Send automation to plugin
			virtual_plugins.values[0]->plugin_server->set_automation(autos, start_auto, end_auto, reverse);
// Render plugin
			virtual_plugins.values[0]->plugin_server->process_realtime(source_len,
																		source_position,
																		size);
		}

// reset all virtual plugins
		for(int i = 0; i < virtual_plugins.total; i++)
		{
			virtual_plugins.values[i]->render_count = 1;
		}
	}
return 0;
}
