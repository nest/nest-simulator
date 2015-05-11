
%% Test script to evaluate effect of spike pairing protocol on synaptic
%% strength of a STDP synapse.
%% Synaptic dynamics for STDP synapses according to Abigail Morrison's
%% STDP model (see stdp_rec.pdf).
%% author: Moritz Helias, april 2006
%%
clear;

deltaT=-50:1:50;

lambda=0.1;
w_init=17.0;
alpha=0.11;
mu=0.4;
tau=20;
delay=1.0;

T = 200;
N = 60;

for i=1:length(deltaT)
    w(i) = stdp(w_init, N, T, alpha, mu, lambda, tau, delay, deltaT(i));
end

plot(deltaT, 100*(w-w_init)/w_init, 'b');
hold on;

w_sim = load('weight.gdf');

plot(w_sim(:,1), 100*(w_sim(:,2)-w_init)/w_init,'r.');
hold off;
xlabel('t_{post} - t_{pre} (ms)');
ylabel('PSC change (%)');