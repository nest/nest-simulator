# -*- coding: utf-8 -*-
#
# neuronview.py
#
# This file is part of NEST.
#
# Copyright (C) 2004 The NEST Initiative
#
# NEST is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# NEST is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with NEST.  If not, see <http://www.gnu.org/licenses/>.

import pygtk

pygtk.require('2.0')
import gtk      # noqa
import pango    # noqa
import gobject  # noqa

from matplotlib.figure import Figure              # noqa
from matplotlib.backends.backend_gtkagg import \
    FigureCanvasGTKAgg as FigureCanvas            # noqa
import matplotlib.gridspec as gridspec            # noqa

import os     # noqa
import nest   # noqa

default_neuron = "iaf_psc_alpha"
default_stimulator = "dc_generator"


class Main():
    def __init__(self):

        self._gladefile = "neuronview.glade"

        self._builder = gtk.Builder()
        self._builder.add_from_file(self._gladefile)
        self._builder.connect_signals(self)

        self._win = self._builder.get_object("mainwindow")
        self._win.resize(900, 700)

        box = self._builder.get_object("box5")
        self._stimulatordictview = DictView()
        self._builder.get_object("scrolledwindow2").add(
            self._stimulatordictview)

        box = self._builder.get_object("box4")
        self._neurondictview = DictView()
        self._builder.get_object("scrolledwindow3").add(self._neurondictview)

        self.populate_comboboxes()

        self._figure = Figure(figsize=(5, 4), dpi=100)
        canvas = FigureCanvas(self._figure)
        canvas.set_size_request(200, 250)
        canvas.show()

        box = self._builder.get_object("box3")
        bg_style = box.get_style().bg[gtk.STATE_NORMAL]
        gtk_color = (bg_style.red_float, bg_style.green_float,
                     bg_style.blue_float)
        self._figure.set_facecolor(gtk_color)
        box.pack_start(canvas)

        self._win.show()
        gtk.main()

    def update_figure(self, spikes, potentials):

        if nest.GetKernelStatus("time") != 0.0:
            self._figure.clear()

            gs = gridspec.GridSpec(2, 1, height_ratios=[1, 4])

            ax0 = self._figure.add_subplot(gs[0])
            ax0.plot(spikes[0]["times"], [1] * len(spikes[0]["times"]), ".")
            ax0.set_yticks([])
            ax0.set_xticks([])

            ax1 = self._figure.add_subplot(gs[1])
            ax1.plot(potentials[0]["times"], potentials[0]["V_m"], "r-")
            ax1.set_ylabel("$V_m$ (mV)")
            ax1.set_xlabel("time (s)")

            #            plt.tight_layout()

            self._figure.canvas.draw()

    def filter_statusdict(self, params):

        for key in ["archiver_length", "available", "capacity",
                    "elementsize", "frozen", "global_id",
                    "instantiations", "is_refractory", "local",
                    "model", "element_type", "offset", "origin",
                    "receptor_types", "recordables",
                    "refractory_input", "rmax", "state", "t_spike",
                    "thread", "tlast", "tspike", "type_id", "vp",
                    "ymod"]:
            if key in params.keys():
                params.pop(key)

    def populate_comboboxes(self):

        neuronmodels = self._builder.get_object("neuronmodels")
        neuronmodelsliststore = neuronmodels.get_model()

        stimulatormodels = self._builder.get_object("stimulatormodels")
        stimulatormodelsliststore = stimulatormodels.get_model()

        neuron_it = None
        stimulator_it = None

        models = nest.Models("nodes")
        models = [x for x in models if
                  x not in ["correlation_detector", "sli_neuron",
                            "iaf_psc_alpha_norec", "parrot_neuron",
                            "parrot_neuron_ps"]]

        for entry in models:

            try:
                entrytype = nest.GetDefaults(entry)["element_type"]
            except:
                entrytype = "unknown"

            if entrytype == "neuron":
                it = neuronmodelsliststore.append([entry])
                if entry == default_neuron:
                    neuron_it = it
            elif entrytype == "stimulator":
                it = stimulatormodelsliststore.append([entry])
                if entry == default_stimulator:
                    stimulator_it = it

        cell = gtk.CellRendererText()

        neuronmodels.pack_start(cell, True)
        neuronmodels.add_attribute(cell, 'text', 0)
        neuronmodels.set_active_iter(neuron_it)

        stimulatormodels.pack_start(cell, True)
        stimulatormodels.add_attribute(cell, 'text', 0)
        stimulatormodels.set_active_iter(stimulator_it)

        docviewcombo = self._builder.get_object("docviewcombo")
        docviewcomboliststore = docviewcombo.get_model()
        docviewcomboliststore.append(["Stimulating device"])
        it = docviewcomboliststore.append(["Neuron"])
        docviewcombo.pack_start(cell, True)
        docviewcombo.add_attribute(cell, 'text', 0)
        docviewcombo.set_active_iter(it)

    def get_help_text(self, name):

        nest.ll_api.sli_run("statusdict /prgdocdir get")
        docdir = nest.ll_api.sli_pop()

        helptext = "No documentation available"

        for subdir in ["cc", "sli"]:
            filename = os.path.join(docdir, "help", subdir, name + ".hlp")
            if os.path.isfile(filename):
                helptext = open(filename, 'r').read()

        return helptext

    def on_model_selected(self, widget):

        liststore = widget.get_model()
        model = liststore.get_value(widget.get_active_iter(), 0)

        statusdict = nest.GetDefaults(model)
        self.filter_statusdict(statusdict)

        if widget == self._builder.get_object("neuronmodels"):
            self._neurondictview.set_params(statusdict)

        if widget == self._builder.get_object("stimulatormodels"):
            self._stimulatordictview.set_params(statusdict)

        self.on_doc_selected(self._builder.get_object("docviewcombo"))

    def on_doc_selected(self, widget):

        liststore = widget.get_model()
        doc = liststore.get_value(widget.get_active_iter(), 0)

        docview = self._builder.get_object("docview")
        docbuffer = gtk.TextBuffer()

        if doc == "Neuron":
            combobox = self._builder.get_object("neuronmodels")

        if doc == "Stimulating device":
            combobox = self._builder.get_object("stimulatormodels")

        liststore = combobox.get_model()
        model = liststore.get_value(combobox.get_active_iter(), 0)

        docbuffer.set_text(self.get_help_text(model))
        docview.set_buffer(docbuffer)

        docview.modify_font(pango.FontDescription("monospace 10"))

    def on_simulate_clicked(self, widget):

        nest.ResetKernel()

        combobox = self._builder.get_object("stimulatormodels")
        liststore = combobox.get_model()
        stimulatormodel = liststore.get_value(combobox.get_active_iter(), 0)
        params = self._stimulatordictview.get_params()
        stimulator = nest.Create(stimulatormodel, params=params)

        combobox = self._builder.get_object("neuronmodels")
        liststore = combobox.get_model()
        neuronmodel = liststore.get_value(combobox.get_active_iter(), 0)
        neuron = nest.Create(neuronmodel,
                             params=self._neurondictview.get_params())

        weight = self._builder.get_object("weight").get_value()
        delay = self._builder.get_object("delay").get_value()
        nest.Connect(stimulator, neuron, weight, delay)

        sd = nest.Create("spike_detector")
        nest.Connect(neuron, sd)

        vm = nest.Create("voltmeter", params={"interval": 0.1})
        nest.Connect(vm, neuron)

        simtime = self._builder.get_object("simtime").get_value()
        nest.Simulate(simtime)

        self.update_figure(nest.GetStatus(sd, "events"),
                           nest.GetStatus(vm, "events"))

    def on_delete_event(self, widget, event):

        self.on_quit(widget)
        return True

    def on_quit(self, project):

        self._builder.get_object("mainwindow").hide()
        gtk.main_quit()


class DictView(gtk.TreeView):
    def __init__(self, params=None):

        gtk.TreeView.__init__(self)
        if params:
            self.params = params
            self.repopulate()

        renderer = gtk.CellRendererText()
        column = gtk.TreeViewColumn("Name", renderer, text=1)
        self.append_column(column)

        renderer = gtk.CellRendererText()
        renderer.set_property("mode", gtk.CELL_RENDERER_MODE_EDITABLE)
        renderer.set_property("editable", True)
        column = gtk.TreeViewColumn("Value", renderer, text=2)
        self.append_column(column)

        self.set_size_request(200, 150)

        renderer.connect("edited", self.check_value)

        self.show()

    def repopulate(self):

        model = gtk.TreeStore(gobject.TYPE_PYOBJECT, gobject.TYPE_STRING,
                              gobject.TYPE_STRING)

        for key in sorted(self.params.keys()):
            pos = model.insert_after(None, None)

            data = {"key": key, "element_type": type(self.params[key])}
            model.set_value(pos, 0, data)
            model.set_value(pos, 1, str(key))
            model.set_value(pos, 2, str(self.params[key]))

        self.set_model(model)

    def check_value(self, widget, path, new_text):

        model = self.get_model()
        data = model[path][0]

        try:

            typename = data["element_type"].__name__
            new_value = eval("%s('%s')" % (typename, new_text))
            if typename == "bool" and new_text.lower() in ["false", "0"]:
                new_value = False
            self.params[data["key"]] = new_value
            model[path][2] = str(new_value)

        except ValueError:

            old_value = self.params[data["key"]]
            model[path][2] = str(old_value)

    def get_params(self):

        return self.params

    def set_params(self, params):

        self.params = params
        self.repopulate()


if __name__ == "__main__":
    Main()
