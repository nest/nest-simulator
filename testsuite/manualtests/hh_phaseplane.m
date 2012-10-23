%% hh_phaseplane.m
%%
%% numerical phase-plane analysis of the Hodgkin-Huxley neuron.
%% needs data files phaseplane.txt and AP.txt written by
%% test_hh_phaseplane.sli
%% author Sven Schrader, Feb 2007

phasefile='phaseplane.txt';
apfile='AP.txt';

if( (exist(phasefile) ~= 2) | (exist(apfile) ~= 2) )
    error('could not find data file(s). Make sure you have run test_hh_phaseplane.sli!');
end

x=load(phasefile);
t=size(x,1);

V=unique(x(:,1));
n=unique(x(:,2));

r1=abs(reshape(x(:,3),length(n),length(V)));
r2=abs(reshape(x(:,4),length(n),length(V)));
 
nc_v=[];
nc_n=[];
count_v=0;
count_n=0;

f=figure; a=axes; hold on;

nullcline_V=[];
nullcline_n=[];
ncount=1;
vcount=1;

disp('searching nullclines');
for i=1:length(V)
  [dummy index] =  min(r1(:,i));
    if(index ~= 1 & index~=length(n))
      nullcline_V(vcount,:)= [V(i), n(index)];
      vcount=vcount+1;
  end
 
  [dummy index] =  min(r2(:,i));
  if(index ~= 1 & index~=length(n))
      nullcline_n(ncount,:)= [V(i), n(index)];
      ncount=ncount+1;
  end
  
end

disp('plotting vector field')
factor=0.1;
for i=1:3:t
    plot([x(i,1) x(i,1)+factor*x(i,3)], [x(i,2)  x(i,2)+factor*x(i,4)],'COLOR',[0.6 0.6 0.6]);
end

plot(nullcline_V(:,1), nullcline_V(:,2),'r','linewidth',2);
plot(nullcline_n(:,1), nullcline_n(:,2),'b','linewidth',2);

xlim([V(1) V(end)])
ylim([n(1) n(end)])

ap=load('AP.txt');
plot(ap(:,1),ap(:,2),'k','linewidth',1 )

% due to a 'legend' problem, the plot-order needs to be changed
ch=get(a,'Children');
set(a,'Children', flipud(ch))
legend('action potential','n-nullcline','V-nullcline');
set(a,'Children', (ch))

box on
set(a,'Layer','top')

xlabel('membrane potential V (mV)')
ylabel('inactivation variable n')
title('Phase space of the Hodgkin-Huxley Neuron')
