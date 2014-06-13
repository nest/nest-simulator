datadir=@PKGDOCDIR@/examples

nobase_data_DATA=\
	balancedneuron.sli\
	balancedneuron-2.sli\
	brunel-2000.sli\
	brunel-2000_threaded_connect.sli\
	brunel_ps.sli\
	Brette_et_al_2007/coba.sli\
	Brette_et_al_2007/cuba.sli\
	Brette_et_al_2007/cuba_stdp.sli\
	Brette_et_al_2007/cuba_ps.sli\
	Brette_et_al_2007/hh_coba.sli\
	Brette_et_al_2007/benchmark.sli\
	multimeter.sli\
	music/clocktest.music\
	music/conttest.music\
	music/conttest.py\
	music/eventtest.music\
	music/messages0.dat\
	music/messages1.dat\
	music/minimalmusicsetup.music\
	music/minimalmusicsetup-pynest.music\
	music/minimalmusicsetup_receivenest.py\
	music/minimalmusicsetup_receivenest.sli\
	music/minimalmusicsetup_sendnest.py\
	music/minimalmusicsetup_sendnest.sli\
	music/msgtest.music\
	music/msgtest.py\
	music/spike_exporter.sli\
	music/three_neurons_threaded.music\
	music/three_neurons_threaded_receivenest.sli\
	music/three_neurons_threaded_sendnest.sli

install-data-hook:
	chmod 755 $(DESTDIR)@PKGDOCDIR@/examples/music/conttest.py
	chmod 755 $(DESTDIR)@PKGDOCDIR@/examples/music/minimalmusicsetup_receivenest.py
	chmod 755 $(DESTDIR)@PKGDOCDIR@/examples/music/minimalmusicsetup_sendnest.py
	chmod 755 $(DESTDIR)@PKGDOCDIR@/examples/music/msgtest.py
