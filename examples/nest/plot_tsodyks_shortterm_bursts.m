%%
%% Matlab script to evaluate simulation of a network with
%% short-term-plastic synapses according to
%% Misha Tsodyks, Asher Uziel, and Henry Markram
%% "Synchrony generation in Recurrent Networks with Frequency-Dependent
%% Synapses.", Journal of Neuroscience (2000), Vol. 20 RC50 1-5.
%%
%% This script plots
%%
%% 1.) (top left) Dot display of all spikes. Neurons are sorted by their
%%     background input current in ascending order.
%% 2.) (top right) The fraction of active synapses during 1 ms
%%     (population activity)
%% 3.) (bottom left) The firing rates of all excitatory neurons sorted
%%     in ascending order.
%%
%% author: Moritz Helias
%% date: may 2006

clear all;

spikes = load('spike_detector-0-0-503.gdf');

figure (1);

h = 0.25;
t_start = 0;
t_end = 10000;

Ne = 400;
Ni = 100;

tau_m = 30;
C = 30.0;
R = tau_m/C;

tau_r = 3.0;

Vreset = 13.5;
Theta = 15.0;

deltaI = 1.0;
I0 = Theta/R;

indices = find( (spikes(:,4)>=t_start/h) & (spikes(:,4)<=t_end/h) );

spike_section = spikes(indices,:);

%% top left: dotdisplay, only plot every fifth neuron
subplot(2,2,1);
indices = find(mod(spike_section(:,3),5)==0);
ps = spike_section(indices,:);
plot( ps(:,4)*h, ps(:,3) + (ps(:,2)-1) * Ne, '.' );
xlabel('time/ms');
ylabel('neuron nr');

%% top right: population activity
%% binning: 1ms
edges = t_start/h:1.0/h:t_end/h;
act = histc(spike_section(:,4), edges)/(Ne+Ni);
subplot(2,2,2);
plot(edges(:)*h,act(:));
xlabel('time/ms');
ylabel('rel. active neurons');

%% find all excitatory neurons
exc_neurons = spikes(find(spikes(:,2)==1),:);

% mean isi of each exc neuron
Ttot = (max(spikes(:,4))-min(spikes(:,4)))*h;
for n=1:Ne
    exc_T(n) = Ttot/length(find(exc_neurons(:,3)==n));
end
exc_T_sorted = sort(exc_T);

%% bottom left: rates of exc. neurons in ascending order
subplot(2,2,3);
exc_rate=1000./exc_T_sorted;
plot(Ne:-1:1, exc_rate);
xlabel('exc. neuron nr');
ylabel('maen firing rate/Hz');