
%% Synaptic dynamics for STDP synapses according to Abigail Morrison's
%% STDP model (see stdp_rec.pdf).
%% author: Moritz Helias, april 2006
%%
function [w]=stdp(w_init, N, T, alpha, mu, lambda, tau, delay, delta_t)
    w = w_init;
    
    K_plus=0.0;
    K_minus=0.0;
    
    % take into accout dendritic delay   
    delta_t = delta_t + delay;
    
    if (delta_t > 0) % post after pre
        dt_plus = delta_t;
        dt_minus = T-delta_t;
    else
        dt_plus = T+delta_t; % pre after post
        dt_minus = -delta_t;
    end
    
    % repeat for N spike pairs
    for i=1:N
                        
        if (not(delta_t==0))
        
            % here t = t_spike_post
            % presyn spike was dt_plus ago
            K_plus = K_plus*exp(-dt_plus/tau);       
            K_minus = K_minus*exp(-dt_plus/tau);                
            
            w = w + lambda * w^mu * K_plus;
            
            K_minus = K_minus + 1.0;
            
            % here t = t_spike_pre
            % postsyn spike was dt_minus ago                        
            K_minus = K_minus*exp(-dt_minus/tau);
            K_plus = K_plus*exp(-dt_minus/tau);
            
            w = w - lambda * alpha * w * K_minus;
                        
            K_plus = K_plus + 1.0;
            
        end
        
    end

    w 
end