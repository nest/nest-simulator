# -*- coding: utf-8 -*-
#
# test_fixed_multi_cmpt_neu.py
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

# This script tests the iaf_cond_alpha_mc_fixedca_neuron in NEST, 
# while comparing it with an implementation of the neuron model in
# python using ode solver. The inputs to both neuron models are spikes
# to an excitatory synapse at the distal compartment to trigger a calcium
# spike. The membrane potential at the distal compartment of the models
# are then compared.


import numpy as np
import pylab as pl
import matplotlib.cm as cm
import math
import nest
from scipy import *
from scipy.integrate import ode
import unittest

HAVE_GSL = nest.sli_func("statusdict/have_gsl ::")

@unittest.skipIf(not HAVE_GSL, 'GSL is not available')
@nest.check_stack
class mc_neuron_ode():


	def fun_u(self,t,y):
		
		f = [1./self.C_m[0,0]*(-self.G_l[0,0]*(y[0]-self.E_l[0,0]) - y[12]*(y[0]-self.E_ex[0,0]) - y[14]*(y[0]-self.E_in[0,0]) - self.G_conn[0]*((y[0] - self.E_l[0,0])-(y[1] - self.E_l[1,0])) + self.I_curr[0,0]),\
                     1./self.C_m[1,0]*(-self.G_l[1,0]*(y[1]-self.E_l[1,0]) - y[8]*(y[1]-self.E_ex[1,0]) - y[10]*(y[1]-self.E_in[1,0]) - self.G_conn[0]*((y[1] - self.E_l[1,0])-(y[0] - self.E_l[0,0])) - self.G_conn[1]*((y[1] - self.E_l[1,0])-(y[2] - self.E_l[2,0])) + self.I_curr[1,0]),\
                     1./self.C_m[2,0]*(-self.G_l[2,0]*(y[2]-self.E_l[2,0]) - y[4]*(y[2]-self.E_ex[2,0]) - y[6]*(y[2]-self.E_in[2,0]) - self.G_conn[1]*((y[2] - self.E_l[2,0])-(y[1] - self.E_l[1,0])) + self.I_curr[2,0] + self.catmp),\
		     -y[3]/self.Tau_synE[2,0],\
		     y[3] - y[4]/self.Tau_synE[2,0],\
		     -y[5]/self.Tau_synI[2,0],\
		     y[5] - y[6]/self.Tau_synI[2,0],\
		     -y[7]/self.Tau_synE[1,0],\
		     y[7] - y[8]/self.Tau_synE[1,0],\
		     -y[9]/self.Tau_synI[1,0],\
		     y[9] - y[10]/self.Tau_synI[1,0],\
		     -y[11]/self.Tau_synE[0,0],\
		     y[11] - y[12]/self.Tau_synE[0,0],\
		     -y[13]/self.Tau_synI[0,0],\
		     y[13] - y[14]/self.Tau_synI[0,0]]

	
		return f
	

		

	def __init__(self,active,resolution):
		
		self.num_cmpt = 3
		self.cmpt_cnt = 0

		self.active_flag = active
		self.resolution = resolution
		self.V_th = -45.0         		   #mV
		self.dist_V_th = -20.0
		self.V_T = self.V_th
		self.dist_VT = self.dist_V_th
		self.V_reset = -60.0                    #mV
	    	self.T_ref = 2.0                        #ms
		self.G_conn = [25.,25.]		   #nS

		self.tau_VT = 7.
		self.VT_jump = 25.
		self.reset_time = int(self.T_ref/self.resolution)
		self.tau_distVT = 20.
		self.dist_VT_jump = 20.
		self.counter = 0
		self.ref = 0
	    	self.E_l = np.array([[-70.0],[-65.0],[-60.0]])          #mV
		self.Conn_mat = np.array([[self.G_conn[0],-self.G_conn[0],0.],[-self.G_conn[0],self.G_conn[0]+self.G_conn[1]+self.E_l[1],-self.G_conn[1]],[0.,-self.G_conn[1],self.G_conn[1]+self.E_l[2]]])
		self.G_l = np.array([[5.0],[5.0],[5.0]])                #nS
	    	self.C_m = np.array([[43.0],[18.0],[13.0]])           #pF

	    	self.E_ex = np.array([[0.0],[0.0],[0.0]])               #mV
	    	self.E_in = np.array([[-85.0],[-85.0],[-85.0]])         #mV

	    	self.Tau_synE = np.array([[1.0],[1.0],[1.0]])           #ms
	    	self.Tau_synI = np.array([[2.0],[2.0],[2.0]])           #ms
		self.u = np.array([-70.0,-65.0,-60.0,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.]) 
		self.G_exc = np.array([[0.0],[0.0],[0.0]])
		self.G_inh = np.array([[0.0],[0.0],[0.0]])
		self.G_ca = np.array([[0.0],[0.0],[0.0]])
		self.G_exc1 = np.array([[0.0],[0.0],[0.0]])
		self.G_inh1 = np.array([[0.0],[0.0],[0.0]])
		self.G_ca1 = np.array([[0.0],[0.0],[0.0]])
		self.I_conn = 0.0
		self.I_curr = np.array([[0.0],[0.0],[0.0]])

		self.y_exc = np.array([[[0.],[0.]],[[0.],[0.]],[[0.],[0.]]])
		self.y_inh = np.array([[[0.],[0.]],[[0.],[0.]],[[0.],[0.]]])
		self.y_ca  = np.array([[[0.],[0.]],[[0.],[0.]],[[0.],[0.]]])

		# distal_spike parameters
		self.thca = -24.5
		self.ca = np.array([0.113805061345,2.57207665232,18.02752317,48.114851194,85.6436354848,128.220315409,173.216927443,218.423863108,262.925421389,306.489717242,349.112491753,390.843755892,431.733619358,471.823738396,511.150042808,549.738877611,587.609414528,624.775717584,661.244954807,697.018244723,732.094555251,766.472879664,800.153928882,833.134755215,865.400840475,896.94413699,927.753702162,957.824119589,987.161765639,1015.76828911,1043.64028973,1070.77212765,1097.15959686,1122.79624734,1147.67503772,1171.78927916,1195.13449965,1217.7113385,1239.5229808,1260.57216866,1280.85856267,1300.3823753,1319.15042263,1337.17144501,1354.45296787,1371.0056946,1386.84110397,1401.97297186,1416.41611455,1430.18572133,1443.30020284,1455.77646823,1467.62979231,1478.87540735,1489.53098862,1499.61782948,1509.15858215,1518.18231644,1526.71933347,1534.79386441,1542.42560126,1549.62754258,1556.41320802,1562.80746263,1568.83615284,1574.52140349,1579.87205151,1584.90454074,1589.64818584,1594.12204491,1598.34252662,1602.32154243,1606.06491015,1609.57660112,1612.86777674,1615.95719494,1618.86108026,1621.58734276,1624.14441992,1626.54333639,1628.80052582,1630.9262585,1632.92331089,1634.79507018,1636.54980257,1638.1963792,1639.74243747,1641.19678003,1642.56151916,1643.83507239,1645.01794436,1646.12227113,1647.15426479,1648.10980187,1648.98687404,1649.78430366,1650.506708,1651.1617283,1651.75765513,1652.30286116,1652.79955215,1653.24692827,1653.64575826,1653.99835872,1654.31053409,1654.58566801,1654.8259252,1655.02928695,1655.19449773,1655.3299823,1655.43907779,1655.52127023,1655.57987024,1655.61156327,1655.61431701,1655.59470485,1655.55412535,1655.49136896,1655.41100313,1655.31508708,1655.20870781,1655.09962685,1654.98805189,1654.87307229,1654.75139255,1654.61465196,1654.45770978,1654.29007684,1654.11257118,1653.91887835,1653.70425318,1653.45744893,1653.18186443,1652.8856851,1652.56911548,1652.2287645,1651.85646771,1651.44984549,1651.01563677,1650.56070426,1650.08573735,1649.58626069,1649.0607038,1648.50634931,1647.92782738,1647.32896996,1646.70424845,1646.05080423,1645.36962204,1644.67248825,1643.96631617,1643.25143872,1642.52926104,1641.80219514,1641.0717279,1640.33917833,1639.6093511,1638.87832674,1638.14136141,1637.39793101,1636.64628233,1635.88275776,1635.10838145,1634.32434556,1633.52506389,1632.71057285,1631.88582239,1631.04912535,1630.20501515,1629.34778326,1628.4693724,1627.5749934,1626.67293805,1625.76423134,1624.84081167,1623.90129996,1622.94719813,1621.96371022,1620.94872731,1619.91464871,1618.86877358,1617.81445818,1616.7536454,1615.68686916,1614.60677091,1613.51653525,1612.42279632,1611.32285649,1610.21734915,1609.10908765,1607.99625335,1606.87207165,1605.73367114,1604.58580718,1603.42745263,1602.2563389,1601.07405087,1599.8771964,1598.66725182,1597.45119976,1596.23192728,1595.00788329,1593.77635136,1592.53388949,1591.27629698,1590.008634,1588.73801819,1587.46355843,1586.1850879,1584.91018259,1583.64109198,1582.37847784,1581.12667289,1579.88719222,1578.65890834,1577.43698358,1576.2162045,1574.99217227,1573.76505085,1572.5375206,1571.31204754,1570.0878171,1568.86517202,1567.6474138,1566.43162693,1565.21423353,1563.99183704,1562.76240574,1561.5232335,1560.2751458,1559.01817771,1557.75002902,1556.46721769,1555.16738867,1553.85937197,1552.55589151,1551.26127734,1549.96145207,1548.64825828,1547.32689751,1546.00077877,1544.67944741,1543.36605585,1542.05691775,1540.75010129,1539.44722941,1538.15583329,1536.87985316,1535.61770495,1534.36896172,1533.13286062,1531.90356334,1530.67737625,1529.45525602,1528.23397786,1527.01091411,1525.78760429,1524.56412504,1523.34084405,1522.11892724,1520.89548514,1519.66927907,1518.44116777,1517.21063358,1515.97994774,1514.75098915,1513.52240878,1512.28645729,1511.04303186,1509.80350479,1508.57332243,1507.36125266,1506.17088331,1504.99810118,1503.83845515,1502.68693667,1501.54541383,1500.41326892,1499.28749465,1498.16453038,1497.04564003,1495.93024774,1494.81274416,1493.6893789,1492.5524788,1491.39832146,1490.23085431,1489.0538837,1487.86579307,1486.66487283,1485.45347166,1484.22891203,1482.98601166,1481.72458075,1480.44970584,1479.16851642,1477.88321317,1476.59342441,1475.30049486,1474.00590424,1472.70776447,1471.40315358,1470.08822392,1468.75969081,1467.41867341,1466.06814632,1464.71142785,1463.34992263,1461.98438729,1460.62094438,1459.26178099,1457.90429823,1456.54511135,1455.1801917,1453.80638424,1452.42156682,1451.03020819,1449.6356229,1448.24151434,1446.85217648,1445.47091509,1444.09740672,1442.72766693,1441.35912846,1439.9887615,1438.61352754,1437.2331003,1435.85064176,1434.46629022,1433.07871917,1431.6886702,1430.29137743,1428.88383948,1427.46880221,1426.05012411,1424.63112382,1423.20986972,1421.78204359,1420.3422579,1418.88326679,1417.40963166,1415.92758071,1414.44277891,1412.94930739,1411.44221209,1409.93356481,1408.43556623,1406.95473968,1405.48930326,1404.03388589,1402.58211218,1401.13246958,1399.68629963,1398.24342848,1396.80279203,1395.36001385,1393.91173814,1392.4590354,1391.00214462,1389.54010791,1388.07090953,1386.59221586,1385.10237303,1383.60182035,1382.09375947,1380.58218071,1379.07455417,1377.57672104,1376.086262,1374.59961093,1373.1167372,1371.63857108,1370.16297319,1368.69158863,1367.22781199,1365.76831096,1364.31151068,1362.8558221,1361.40016764,1359.94374674,1358.48544867,1357.02300401,1355.55682408,1354.08658711,1352.60968448,1351.1287602,1349.64512065,1348.15846282,1346.66616782,1345.16607855,1343.65835141,1342.14511175,1340.63014399,1339.09304742,1337.52637112,1335.95364894,1334.38223006,1332.81103138,1331.24772858,1329.69653854,1328.15433861,1326.61972993,1325.08796029,1323.55438224,1322.02219535,1320.49192177,1318.96169113,1317.432706,1315.90255946,1314.36452089,1312.81422679,1311.25332637,1309.6868102,1308.11747685,1306.54344907,1304.95907624,1303.3602152,1301.74390309,1300.10775443,1298.45263913,1296.78007612,1295.09234583,1293.39019508,1291.67421399,1289.94735243,1288.21191536,1286.46623999,1284.70920124,1282.9442224,1281.17476606,1279.40212508,1277.6261545,1275.84825096,1274.06887318,1272.28710816,1270.49928505,1268.70111065,1266.89434131,1265.08021394,1263.25626088,1261.42223417,1259.57976379,1257.72640146,1255.85583308,1253.96578733,1252.05619553,1250.12216168,1248.16184243,1246.18013248,1244.181028,1242.16320398,1240.12000691,1238.05043141,1235.95775879,1233.84414395,1231.6957699,1229.50713194,1227.28794078,1225.04307789,1222.77937855,1220.49908075,1218.2026686,1215.89380054,1213.57070931,1211.23033824,1208.87209207,1206.49299239,1204.09217185,1201.67079388,1199.22962621,1196.76561776,1194.27710631,1191.76417554,1189.22611967,1186.66364254,1184.08013788,1181.47942354,1178.86286818,1176.2289322,1173.57646846,1170.90327413,1168.20606748,1165.48449396,1162.73748258,1159.96248104,1157.15674466,1154.31525823,1151.43582179,1148.51534501,1145.54854552,1142.5327673,1139.46362641,1136.34208875,1133.17097716,1129.95146939,1126.68299937,1123.36541991,1120.00225004,1116.5937351,1113.13888039,1109.63564865,1106.07958083,1102.46792812,1098.79952731,1095.07299489,1091.28561738,1087.43493899,1083.51887658,1079.5348956,1075.48033446,1071.32732558,1067.06869205,1062.72930984,1058.32466592,1053.86115015,1049.33900677,1044.7574558,1040.11249238,1035.39958451,1030.61636236,1025.76354374,1020.84198402,1015.8476865,1010.77538298,1005.62473733,1000.40048663,995.104132885,989.73470897,984.29114096,978.773538198,973.183389307,967.521569932,961.78879934,955.986355001,950.115204937,944.176200299,938.171477924,932.103194002,925.973667667,919.784231302,913.534581861,907.225299968,900.856497155,894.428840483,887.943238631,881.401389467,874.805273935,868.157658364,861.464050917,854.72781196,847.950485999,841.133597625,834.279419612,827.389833812,820.465519764,813.508343419,806.518633912,799.497169344,792.446067886,785.365607854,778.256970907,771.123052361,763.967431609,756.793081666,749.601866199,742.398065448,735.187327767,727.974187436,720.760538633,713.547585987,706.338677913,699.135959944,691.940482755,684.754613364,677.577166216,670.40730965,663.247923726,656.100334501,648.966895917,641.849999549,634.749605354,627.666781095,620.603102944,613.559916268,606.538438814,599.542383908,592.576381439,585.642653631,578.74260529,571.878194927,565.051380612,558.263724021,551.516912156,544.81201153,538.150168265,531.532684345,524.960582134,518.434149438,511.954664338,505.524413869,498.938531257,492.737790076,486.537048896,480.336307715,474.135566535,468.081244968,462.026923401,455.972601834,449.918280267,444.087144368,438.256008468,432.424872569,426.59373667,420.986676969,415.379617269,409.772557569,404.165497869,398.771247521,393.376997172,387.982746824,382.588496475,377.407850654,372.227204832,367.04655901,361.865913188,356.901880739,351.937848291,346.973815842,342.009783394,337.258138801,332.506494208,327.754849615,323.003205022,318.454323491,313.90544196,309.35656043,304.807678899,300.464516029,296.12135316,291.77819029,287.435027421,283.387015958,279.339004494,275.290993031,271.242981568,267.194970105,263.146958641,259.098947178,255.050935715,251.417348255,247.783760795,244.150173335,240.516585876,236.882998416,233.249410956,229.615823496,225.982236036,222.689902822,219.397569607,216.105236392,212.812903178,209.520569963,206.228236748,202.935903534,199.957443589,196.978983644,194.000523698,191.022063753,188.043603808,185.065143863,182.086683918,179.108223973,176.452216715,173.796209457,171.140202199,168.484194942,165.828187684,163.172180426,160.516173168,157.86016591,155.490160205,153.1201545,150.750148795,148.38014309,146.010137384,143.640131679,141.270125974,138.900120269,136.775497915,134.650875561,132.526253208,130.401630854,128.2770085,126.152386146,124.027763793,122.062860176,120.09795656,118.133052943,116.168149326,114.313064442,112.457979557,110.602894672,108.747809787,107.049982604,105.352155421,103.654328238,101.956501055,100.258673872,98.5608466894,96.8630195064,95.1651923235,93.6718636897,92.1785350559,90.685206422,89.1918777882,87.6985491544,86.2052205206,84.7118918868,83.218563253,81.8922722108,80.5659811687,79.2396901266,77.9133990844,76.5871080423,75.2608170002,73.934525958,72.7129223239,71.4913186898,70.2697150557,69.0481114216,67.8979621885,66.7478129555,65.5976637225,64.4475144894,63.4001803863,62.3528462832,61.30551218,60.2581780769,59.2108439738,58.1635098707,57.1161757675,56.0688416644,55.1557095851,54.2425775059,53.3294454266,52.4163133473,51.5031812681,50.5900491888,49.6769171095,48.7637850303,47.9584066843,47.1530283384,46.3476499925,45.5422716466,44.7368933007,43.9315149548,43.1261366088,42.4160985878,41.7060605668,40.9960225458,40.2859845248,39.5759465038,38.8659084828,38.1558704618,37.4458324408,36.825016781,36.2042011213,35.5833854615,34.9625698017,34.3417541419,33.7209384822,33.1001228224,32.4793071626,31.9492444826,31.4191818027,30.8891191227,30.3590564427,29.8289937627,29.2989310828,28.7688684028,28.2388057228,27.7916491298,27.3444925367,26.8973359437,26.4501793506,26.0030227576,25.5558661645,25.1087095715,24.778410471,24.4481113705,24.11781227,23.7875131695,23.457214069,23.1269149685,22.796615868,22.4663167675,22.136017667,21.8057185665,21.475419466,21.1451203655,20.814821265,20.4845221645,20.154223064,19.8239239635,19.5823199452,19.340715927,19.0991119087,18.8575078905,18.6159038722,18.3742998539,18.1326958357,17.8910918174,17.6494877992,17.4078837809,17.1662797627,16.9246757444,16.6830717262,16.4414677079,16.1998636897,16.0215306927,15.8431976957,15.6648646987,15.4865317017,15.3081987047,15.1298657077,14.9515327107,14.7731997137,14.5948667167,14.4165337197,14.2382007227,14.0598677257,13.8815347287,13.7032017317,13.5248687347,13.3465357377,13.2033478434,13.0601599491,12.9169720549,12.7737841606,12.6305962663,12.487408372,12.3442204777,12.2010325835,12.0578446892,11.9146567949,11.7714689006,11.6282810064,11.4850931121,11.3419052178,11.1987173235,11.0555294292,10.912341535,10.7691536407,10.6259657464,10.4827778521,10.3395899578,10.1964020636,10.0532141693,9.91002627501,9.76683838073,9.62365048645,9.48046259217,9.33727469789,9.19408680361,9.05089890934,8.90771101506,8.76452312078,8.6213352265,8.47814733222,8.33495943794,8.19177154366,8.04858364938,7.90539575511,7.76220786083,7.61901996655,7.47583207227,7.33264417799,7.18945628371,7.04626838943,6.90308049515,6.75989260088,6.6167047066,6.47351681232,6.33032891804,6.18714102376,6.04395312948,5.9007652352,5.75757734092,5.61438944664,5.47120155237,5.32801365809,5.18482576381,5.04163786953,4.89844997525,4.75526208097,4.61207418669,4.46888629241,4.32569839814,4.18251050386,4.03932260958,3.8961347153,3.75294682102,3.60975892674,3.46657103246,3.32338313818,3.18019524391,3.03700734963,2.89381945535,2.75063156107,2.60744366679,2.46425577251,2.32106787823,2.17787998395,2.03469208967,1.8915041954,1.74831630112,1.60512840684,1.46194051256,1.31875261828,1.175564724,1.03237682972,0.889188935444,0.746001041165,0.602813146887,0.459625252608,0.316437358329,0.17324946405,0.0300615697713,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0])
		self.refca = 999
		self.catmp = 0.0
		self.reftmp = 0


		self.r = ode(self.fun_u).set_integrator('dop853',rtol=1e-3, nsteps=500)
		self.r.set_initial_value(self.u,0.)
	
	def compute_u(self,G_exc,G_inh,I_curr):



		self.counter = 0
		self.distal_cnt = 0

		self.r.y[11] += (G_exc[0,0]*np.exp(1)/self.Tau_synE[0,0])
		self.r.y[13] += (G_inh[0,0]*np.exp(1)/self.Tau_synI[0,0])
		self.r.y[7] += (G_exc[1,0]*np.exp(1)/self.Tau_synE[1,0])
		self.r.y[9] += (G_inh[1,0]*np.exp(1)/self.Tau_synI[1,0])
		self.r.y[3] += (G_exc[2,0]*np.exp(1)/self.Tau_synE[2,0])
		self.r.y[5] += (G_inh[2,0]*np.exp(1)/self.Tau_synI[2,0])

		self.r.integrate(self.r.t+self.resolution)
		self.u = self.r.y

		if self.reftmp > 0:
			self.reftmp -=1
			self.catmp = self.ca[self.refca - self.reftmp]
		else:
			if self.u[2] >= self.thca:
				self.reftmp = self.refca
				self.catmp = self.ca[0]
		
@unittest.skipIf(not HAVE_GSL, 'GSL is not available')
@nest.check_stack
class FixedMultiCmptNeuTestCase(unittest.TestCase):		

    def test_FixedMultiCmptNeu(self):		

	trial_num = 1

	active_flag = 1
	resolution = 0.1
	sim_time = 1500
	l = int(sim_time/resolution)
	t2 = np.linspace(0.1,1500.,15000)
	rate = 0.0008
	mother_rate = 0.005
	neur = mc_neuron_ode(active_flag,resolution)

	soma_flag = 0
	dist_flag = 0
	input_frac = 1.
	copy_frac = 0.2

	dist_inh_ratio = 1.0


	amp_exc = 200.
	amp_inh = 100.
	tot_exc = 3
	tot_inh = 3
	E_L = -70.
	I_s = np.zeros((3,int(sim_time/resolution)))
	exc_wt = np.ones((tot_exc,1))
	inh_wt = np.ones((tot_inh,1))
	u_arr = np.ones((3,l))
	u_arr[0,:] = u_arr[0,:]*-70.
	u_arr[1,:] = u_arr[1,:]*-65.
	u_arr[2,:] = u_arr[2,:]*-60.


	exc_g_arr = np.zeros((3,l))
	inh_g_arr = np.zeros((3,l))
	ca_arr = np.zeros(l)
	VT_arr = np.ones(l)*-45.
	dist_VT_arr = np.ones(l)*-20.


	leak_g_arr = np.zeros((3,l))

	tmp_exc_cmpt = np.reshape(np.zeros(3),(3,1))
	tmp_inh_cmpt = np.reshape(np.zeros(3),(3,1))
	spike_cnt = 0

	exc_input_arr = np.zeros([tot_exc,l])
	inh_input_arr = np.zeros([tot_inh,l])

	exc_input_arr[2,1000] = 1

	for t in range(0,l):

		tmp_exc = amp_exc*exc_wt*np.reshape(exc_input_arr[:,t],(tot_exc,1))
		tmp_inh = amp_inh*inh_wt*np.reshape(inh_input_arr[:,t],(tot_inh,1))
		tmp_curr = np.reshape(I_s[:,t],(3,1))

		tmp_exc_cmpt[0,0] = (tmp_exc[0,0])
		tmp_exc_cmpt[1,0] = (tmp_exc[1,0])
		tmp_exc_cmpt[2,0] = (tmp_exc[2,0])
		tmp_inh_cmpt[0,0] = tmp_inh[0,0]
		tmp_inh_cmpt[1,0] = tmp_inh[1,0]
		tmp_inh_cmpt[2,0] = tmp_inh[2,0]
	
	
		neur.compute_u(tmp_exc_cmpt,tmp_inh_cmpt,tmp_curr)
		u_arr[:,t] = neur.u[0:3]
		exc_g_arr[2,t] = neur.u[4]
		inh_g_arr[2,t] = neur.u[6]
		exc_g_arr[1,t] = neur.u[8]
		inh_g_arr[1,t] = neur.u[10]
		exc_g_arr[0,t] = neur.u[12]
		inh_g_arr[0,t] = neur.u[14]

		leak_g_arr[:,t] = np.reshape(neur.G_l,(1,3))



	c_p = 18.
	glp = 5.
	gpd = 25.
	gsp = 25.

	gld = 5.
	gls = 5.
	c_d = 13.
	c_s = 43.
	eld = -60.
	elp = -65.
	els = -70.
	leak = -60.000004268845181
	leak = -60.00000001


	nest.ResetKernel()

	# Obtain receptor dictionary
	syns = nest.GetDefaults('iaf_cond_alpha_mc_fixedca')['receptor_types']
	#print "iaf_cond_alpha_mc receptor_types: ", syns

	# Obtain list of recordable quantities
	rqs = nest.GetDefaults('iaf_cond_alpha_mc_fixedca')['recordables']
	#print "iaf_cond_alpha_mc recordables   : ", rqs

	# Change some default values:
	#  - threshold potential
	#  - reset potential
	#  - refractory period
	#  - somato-proximal coupling conductance
	#  - somatic leak conductance
	#  - proximal synaptic time constants
	#  - distal capacitance


	nest.SetDefaults('iaf_cond_alpha_mc_fixedca',
		 { 
		   'g_sp' : gsp,
		   'g_pd' : gpd,
		   'V_th' : 955.0,
		   'Ca_active' : True,
		   'jump_Th' : 0.,
		   'tau_Th' : 1.,
		   'distal'  : { 
				 't_L' : gld,
				 'nt_L' : gld,
				 'E_L' : eld,
				 'tau_syn_ex': 1.0,
				 'tau_syn_in': 2.0,
				 'g_L': gld,
				 'C_m': c_d,
				 'amp_curr_AP': 0.,
				 'tau_curr_AP': 1. },
		   'proximal'  : { 
	  		           't_L' : glp,
				   'nt_L' : glp,
				   'E_L' : elp,
				   'tau_syn_ex': 1.0,
				   'tau_syn_in': 2.0,
				   'g_L': glp, 
				   'C_m': c_p,
				   'amp_curr_AP': 0.,
				   'tau_curr_AP': 1.  },
		   'soma'  : { 
			       't_L' : 150.,
	  		       'nt_L' : gls,
			       'E_L' : els,
			       'tau_syn_ex': 1.0,
			       'tau_syn_in': 2.0,
			       'g_L': gls, 
			       'C_m': c_s,
			       'amp_curr_AP': 0.,
			       'tau_curr_AP': 1. },
		 }) 



	# Create neuron
	n = nest.Create('iaf_cond_alpha_mc_fixedca')

	# Create multimeter recording everything, connect
	mm = nest.Create('multimeter', 
			 params = {'record_from': rqs, 
			           'interval': 0.1})
	nest.Connect(mm, n)

	
	sgs = nest.Create('spike_generator', 6)
	nest.SetStatus([sgs[0]],[{'spike_times': [99.0]}]) # distal
	nest.SetStatus([sgs[1]],[{'spike_times': [109.0]}]) # distal
	nest.SetStatus([sgs[2]],[{'spike_times': [499.0]}]) # distal
	nest.SetStatus([sgs[3]],[{'spike_times': [509.0]}]) # distal
	nest.SetStatus([sgs[4]],[{'spike_times': [899.0]}]) # distal
	nest.SetStatus([sgs[5]],[{'spike_times': [909.0]}]) # distal



	# Connect generators to correct compartments
	nest.Connect([sgs[0]], n, syn_spec = {'receptor_type': syns['distal_exc'],"weight":amp_exc, "delay":1.0})


	sd = nest.Create('spike_detector', 1)
	nest.Connect(n,sd)

	# Simulate 
	nest.Simulate(1500)

	rec = nest.GetStatus(mm)[0]['events']
	t1 = rec['times']


	error = np.sqrt((u_arr[2,:14990]-rec['V_m.d'])**2)
        self.assertTrue(np.max(error) < 4.0e-4)


def suite():

    suite = unittest.makeSuite(FixedMultiCmptNeuTestCase,'test')
    return suite

def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()


