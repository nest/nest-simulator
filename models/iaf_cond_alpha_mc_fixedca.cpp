/*
 *  iaf_cond_alpha_mc_fixedca.cpp
 *
 *  This file is part of NEST.
 *
 *  Copyright (C) 2004 The NEST Initiative
 *
 *  NEST is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  NEST is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with NEST.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#include "iaf_cond_alpha_mc_fixedca.h"

#ifdef HAVE_GSL

// C++ includes:
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <limits>
#include <math.h>

// Includes from libnestutil:
#include "numerics.h"

// Includes from nestkernel:
#include "exceptions.h"
#include "kernel_manager.h"
#include "universal_data_logger_impl.h"

// Includes from sli:
#include "dict.h"
#include "dictutils.h"
#include "doubledatum.h"
#include "integerdatum.h"


/* ----------------------------------------------------------------
 * Compartment name list
 * ---------------------------------------------------------------- */

/* Harold Gutch reported some static destruction problems on OSX 10.4.
   He pointed out that the problem is avoided by defining the comp_names_
   vector with its final size. See also #348.
*/
std::vector< Name > nest::iaf_cond_alpha_mc_fixedca::comp_names_( NCOMP );

/* ----------------------------------------------------------------
 * Receptor dictionary
 * ---------------------------------------------------------------- */

// leads to seg fault on exit, see #328
// DictionaryDatum nest::iaf_cond_alpha_mc_fixedca::receptor_dict_ = new Dictionary();

/* ----------------------------------------------------------------
 * Recordables map
 * ---------------------------------------------------------------- */

nest::RecordablesMap< nest::iaf_cond_alpha_mc_fixedca >
  nest::iaf_cond_alpha_mc_fixedca::recordablesMap_;

namespace nest
{
// specialization must be place in namespace

template <>
void
RecordablesMap< iaf_cond_alpha_mc_fixedca >::create()
{
  insert_( Name( "V_m.s" ),
    &iaf_cond_alpha_mc_fixedca::get_y_elem_< iaf_cond_alpha_mc_fixedca::State_::V_M,
      iaf_cond_alpha_mc_fixedca::SOMA > );
  insert_( Name( "g_ex.s" ),
    &iaf_cond_alpha_mc_fixedca::get_y_elem_< iaf_cond_alpha_mc_fixedca::State_::G_EXC,
      iaf_cond_alpha_mc_fixedca::SOMA > );
  insert_( Name( "g_in.s" ),
    &iaf_cond_alpha_mc_fixedca::get_y_elem_< iaf_cond_alpha_mc_fixedca::State_::G_INH,
      iaf_cond_alpha_mc_fixedca::SOMA > );
  insert_( Name( "i_ap.s" ),
    &iaf_cond_alpha_mc_fixedca::get_y_elem_< iaf_cond_alpha_mc_fixedca::State_::I_AP,
      iaf_cond_alpha_mc_fixedca::SOMA > );

  insert_( Name( "V_m.p" ),
    &iaf_cond_alpha_mc_fixedca::get_y_elem_< iaf_cond_alpha_mc_fixedca::State_::V_M,
      iaf_cond_alpha_mc_fixedca::PROX > );
  insert_( Name( "g_ex.p" ),
    &iaf_cond_alpha_mc_fixedca::get_y_elem_< iaf_cond_alpha_mc_fixedca::State_::G_EXC,
      iaf_cond_alpha_mc_fixedca::PROX > );
  insert_( Name( "g_in.p" ),
    &iaf_cond_alpha_mc_fixedca::get_y_elem_< iaf_cond_alpha_mc_fixedca::State_::G_INH,
      iaf_cond_alpha_mc_fixedca::PROX > );
  insert_( Name( "i_ap.p" ),
    &iaf_cond_alpha_mc_fixedca::get_y_elem_< iaf_cond_alpha_mc_fixedca::State_::I_AP,
      iaf_cond_alpha_mc_fixedca::PROX > );

  insert_( Name( "V_m.d" ),
    &iaf_cond_alpha_mc_fixedca::get_y_elem_< iaf_cond_alpha_mc_fixedca::State_::V_M,
      iaf_cond_alpha_mc_fixedca::DIST > );
  insert_( Name( "g_ex.d" ),
    &iaf_cond_alpha_mc_fixedca::get_y_elem_< iaf_cond_alpha_mc_fixedca::State_::G_EXC,
      iaf_cond_alpha_mc_fixedca::DIST > );
  insert_( Name( "g_in.d" ),
    &iaf_cond_alpha_mc_fixedca::get_y_elem_< iaf_cond_alpha_mc_fixedca::State_::G_INH,
      iaf_cond_alpha_mc_fixedca::DIST > );
  insert_( Name( "i_ap.d" ),
    &iaf_cond_alpha_mc_fixedca::get_y_elem_< iaf_cond_alpha_mc_fixedca::State_::I_AP,
      iaf_cond_alpha_mc_fixedca::DIST > );

  insert_( names::t_ref_remaining, &iaf_cond_alpha_mc_fixedca::get_r_ );
  insert_( names::threshold, &iaf_cond_alpha_mc_fixedca::get_th_ );

  // changes
  insert_( names::ca_spike_count, &iaf_cond_alpha_mc_fixedca::get_ca_ );
  insert_( names::t_refCa_remaining, &iaf_cond_alpha_mc_fixedca::get_rCa_ );
  insert_( names::ca_spike, &iaf_cond_alpha_mc_fixedca::get_ICa_ );
  insert_(names::curr_s, &iaf_cond_alpha_mc_fixedca::get_CurrS_);
  insert_(names::curr_p, &iaf_cond_alpha_mc_fixedca::get_CurrP_);
  insert_(names::curr_d, &iaf_cond_alpha_mc_fixedca::get_CurrD_);
}
}

/* ----------------------------------------------------------------
 * Iteration function
 * ---------------------------------------------------------------- */

extern "C" int
nest::iaf_cond_alpha_mc_fixedca_dynamics( double, const double y[], double f[], void* pnode )
{
  // some shorthands
  typedef nest::iaf_cond_alpha_mc_fixedca N;
  typedef nest::iaf_cond_alpha_mc_fixedca::State_ S;

  // get access to node so we can work almost as in a member function
  assert( pnode );
  const nest::iaf_cond_alpha_mc_fixedca& node =
    *( reinterpret_cast< nest::iaf_cond_alpha_mc_fixedca* >( pnode ) );

  // compute dynamics for each compartment
  // computations written quite explicitly for clarity, assume compile
  // will optimized most stuff away ...
  for ( size_t n = 0; n < N::NCOMP; ++n )
  {
    // membrane potential for current compartment
    const double V = y[ S::idx( n, S::V_M ) ];

    // excitatory synaptic current
    const double I_syn_exc = ( y[ S::idx( n, S::G_EXC ) ] ) * ( V - node.P_.E_ex[ n ] );
    // inhibitory synaptic current
    const double I_syn_inh = ( y[ S::idx( n, S::G_INH ) ] ) * ( V - node.P_.E_in[ n ] );

    // leak current
    const double I_L = node.P_.g_L[ n ] * ( V - node.P_.E_L[ n ] );

    double ica = 0.0;
    if ( n == 2 )
      ica = node.S_.ICa_;

    // coupling currents
    const double I_conn =
      ( n > N::SOMA
          ? node.P_.g_conn[ n - 1 ]
            * ( ( V - node.P_.E_L[ n ] ) - ( y[ S::idx( n - 1, S::V_M ) ] - node.P_.E_L[ n - 1 ] ) )
          : 0 )
      + ( n < N::NCOMP - 1
            ? node.P_.g_conn[ n ] * ( ( V - node.P_.E_L[ n ] )
                                      - ( y[ S::idx( n + 1, S::V_M ) ] - node.P_.E_L[ n + 1 ] ) )
            : 0 );

    // derivatives
    // membrane potential
    f[ S::idx( n, S::V_M ) ] =
      ( -I_L - I_syn_exc - I_syn_inh - I_conn + node.B_.I_stim_[ n ] + node.P_.I_e[ n ] + ica
        + y[ S::idx( n, S::I_AP ) ] ) / node.P_.C_m[ n ];

    // excitatory conductance
    f[ S::idx( n, S::DG_EXC ) ] = -y[ S::idx( n, S::DG_EXC ) ] / node.P_.tau_synE[ n ];
    f[ S::idx( n, S::G_EXC ) ] =
      y[ S::idx( n, S::DG_EXC ) ] - y[ S::idx( n, S::G_EXC ) ] / node.P_.tau_synE[ n ];

    // inhibitory conductance
    f[ S::idx( n, S::DG_INH ) ] = -y[ S::idx( n, S::DG_INH ) ] / node.P_.tau_synI[ n ];
    f[ S::idx( n, S::G_INH ) ] =
      y[ S::idx( n, S::DG_INH ) ] - y[ S::idx( n, S::G_INH ) ] / node.P_.tau_synI[ n ];

    // active current during AP
    f[ S::idx( n, S::DI_AP ) ] = -y[ S::idx( n, S::DI_AP ) ] / node.P_.tau_curAP[ n ];
    f[ S::idx( n, S::I_AP ) ] =
      y[ S::idx( n, S::DI_AP ) ] - y[ S::idx( n, S::I_AP ) ] / node.P_.tau_curAP[ n ];
  }

  return GSL_SUCCESS;
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::iaf_cond_alpha_mc_fixedca::Parameters_::Parameters_()
  : V_th( -55.0 )    // mV
  , V_reset( -60.0 ) // mV
  , t_ref( 2.0 )
  , // ms
  // changes
  // max voltage at spike
  V_max( 30.0 )
  , // mV
  V_thCa( -24.5 )
  , // mV
  amp( 1.0 )
  ,

  // sye 1.0, syi 2.0 (different synaptic time constant, different waveform)
  Ca_amp( { 0.113805061345,
    2.57207665232,
    18.02752317,
    48.114851194,
    85.6436354848,
    128.220315409,
    173.216927443,
    218.423863108,
    262.925421389,
    306.489717242,
    349.112491753,
    390.843755892,
    431.733619358,
    471.823738396,
    511.150042808,
    549.738877611,
    587.609414528,
    624.775717584,
    661.244954807,
    697.018244723,
    732.094555251,
    766.472879664,
    800.153928882,
    833.134755215,
    865.400840475,
    896.94413699,
    927.753702162,
    957.824119589,
    987.161765639,
    1015.76828911,
    1043.64028973,
    1070.77212765,
    1097.15959686,
    1122.79624734,
    1147.67503772,
    1171.78927916,
    1195.13449965,
    1217.7113385,
    1239.5229808,
    1260.57216866,
    1280.85856267,
    1300.3823753,
    1319.15042263,
    1337.17144501,
    1354.45296787,
    1371.0056946,
    1386.84110397,
    1401.97297186,
    1416.41611455,
    1430.18572133,
    1443.30020284,
    1455.77646823,
    1467.62979231,
    1478.87540735,
    1489.53098862,
    1499.61782948,
    1509.15858215,
    1518.18231644,
    1526.71933347,
    1534.79386441,
    1542.42560126,
    1549.62754258,
    1556.41320802,
    1562.80746263,
    1568.83615284,
    1574.52140349,
    1579.87205151,
    1584.90454074,
    1589.64818584,
    1594.12204491,
    1598.34252662,
    1602.32154243,
    1606.06491015,
    1609.57660112,
    1612.86777674,
    1615.95719494,
    1618.86108026,
    1621.58734276,
    1624.14441992,
    1626.54333639,
    1628.80052582,
    1630.9262585,
    1632.92331089,
    1634.79507018,
    1636.54980257,
    1638.1963792,
    1639.74243747,
    1641.19678003,
    1642.56151916,
    1643.83507239,
    1645.01794436,
    1646.12227113,
    1647.15426479,
    1648.10980187,
    1648.98687404,
    1649.78430366,
    1650.506708,
    1651.1617283,
    1651.75765513,
    1652.30286116,
    1652.79955215,
    1653.24692827,
    1653.64575826,
    1653.99835872,
    1654.31053409,
    1654.58566801,
    1654.8259252,
    1655.02928695,
    1655.19449773,
    1655.3299823,
    1655.43907779,
    1655.52127023,
    1655.57987024,
    1655.61156327,
    1655.61431701,
    1655.59470485,
    1655.55412535,
    1655.49136896,
    1655.41100313,
    1655.31508708,
    1655.20870781,
    1655.09962685,
    1654.98805189,
    1654.87307229,
    1654.75139255,
    1654.61465196,
    1654.45770978,
    1654.29007684,
    1654.11257118,
    1653.91887835,
    1653.70425318,
    1653.45744893,
    1653.18186443,
    1652.8856851,
    1652.56911548,
    1652.2287645,
    1651.85646771,
    1651.44984549,
    1651.01563677,
    1650.56070426,
    1650.08573735,
    1649.58626069,
    1649.0607038,
    1648.50634931,
    1647.92782738,
    1647.32896996,
    1646.70424845,
    1646.05080423,
    1645.36962204,
    1644.67248825,
    1643.96631617,
    1643.25143872,
    1642.52926104,
    1641.80219514,
    1641.0717279,
    1640.33917833,
    1639.6093511,
    1638.87832674,
    1638.14136141,
    1637.39793101,
    1636.64628233,
    1635.88275776,
    1635.10838145,
    1634.32434556,
    1633.52506389,
    1632.71057285,
    1631.88582239,
    1631.04912535,
    1630.20501515,
    1629.34778326,
    1628.4693724,
    1627.5749934,
    1626.67293805,
    1625.76423134,
    1624.84081167,
    1623.90129996,
    1622.94719813,
    1621.96371022,
    1620.94872731,
    1619.91464871,
    1618.86877358,
    1617.81445818,
    1616.7536454,
    1615.68686916,
    1614.60677091,
    1613.51653525,
    1612.42279632,
    1611.32285649,
    1610.21734915,
    1609.10908765,
    1607.99625335,
    1606.87207165,
    1605.73367114,
    1604.58580718,
    1603.42745263,
    1602.2563389,
    1601.07405087,
    1599.8771964,
    1598.66725182,
    1597.45119976,
    1596.23192728,
    1595.00788329,
    1593.77635136,
    1592.53388949,
    1591.27629698,
    1590.008634,
    1588.73801819,
    1587.46355843,
    1586.1850879,
    1584.91018259,
    1583.64109198,
    1582.37847784,
    1581.12667289,
    1579.88719222,
    1578.65890834,
    1577.43698358,
    1576.2162045,
    1574.99217227,
    1573.76505085,
    1572.5375206,
    1571.31204754,
    1570.0878171,
    1568.86517202,
    1567.6474138,
    1566.43162693,
    1565.21423353,
    1563.99183704,
    1562.76240574,
    1561.5232335,
    1560.2751458,
    1559.01817771,
    1557.75002902,
    1556.46721769,
    1555.16738867,
    1553.85937197,
    1552.55589151,
    1551.26127734,
    1549.96145207,
    1548.64825828,
    1547.32689751,
    1546.00077877,
    1544.67944741,
    1543.36605585,
    1542.05691775,
    1540.75010129,
    1539.44722941,
    1538.15583329,
    1536.87985316,
    1535.61770495,
    1534.36896172,
    1533.13286062,
    1531.90356334,
    1530.67737625,
    1529.45525602,
    1528.23397786,
    1527.01091411,
    1525.78760429,
    1524.56412504,
    1523.34084405,
    1522.11892724,
    1520.89548514,
    1519.66927907,
    1518.44116777,
    1517.21063358,
    1515.97994774,
    1514.75098915,
    1513.52240878,
    1512.28645729,
    1511.04303186,
    1509.80350479,
    1508.57332243,
    1507.36125266,
    1506.17088331,
    1504.99810118,
    1503.83845515,
    1502.68693667,
    1501.54541383,
    1500.41326892,
    1499.28749465,
    1498.16453038,
    1497.04564003,
    1495.93024774,
    1494.81274416,
    1493.6893789,
    1492.5524788,
    1491.39832146,
    1490.23085431,
    1489.0538837,
    1487.86579307,
    1486.66487283,
    1485.45347166,
    1484.22891203,
    1482.98601166,
    1481.72458075,
    1480.44970584,
    1479.16851642,
    1477.88321317,
    1476.59342441,
    1475.30049486,
    1474.00590424,
    1472.70776447,
    1471.40315358,
    1470.08822392,
    1468.75969081,
    1467.41867341,
    1466.06814632,
    1464.71142785,
    1463.34992263,
    1461.98438729,
    1460.62094438,
    1459.26178099,
    1457.90429823,
    1456.54511135,
    1455.1801917,
    1453.80638424,
    1452.42156682,
    1451.03020819,
    1449.6356229,
    1448.24151434,
    1446.85217648,
    1445.47091509,
    1444.09740672,
    1442.72766693,
    1441.35912846,
    1439.9887615,
    1438.61352754,
    1437.2331003,
    1435.85064176,
    1434.46629022,
    1433.07871917,
    1431.6886702,
    1430.29137743,
    1428.88383948,
    1427.46880221,
    1426.05012411,
    1424.63112382,
    1423.20986972,
    1421.78204359,
    1420.3422579,
    1418.88326679,
    1417.40963166,
    1415.92758071,
    1414.44277891,
    1412.94930739,
    1411.44221209,
    1409.93356481,
    1408.43556623,
    1406.95473968,
    1405.48930326,
    1404.03388589,
    1402.58211218,
    1401.13246958,
    1399.68629963,
    1398.24342848,
    1396.80279203,
    1395.36001385,
    1393.91173814,
    1392.4590354,
    1391.00214462,
    1389.54010791,
    1388.07090953,
    1386.59221586,
    1385.10237303,
    1383.60182035,
    1382.09375947,
    1380.58218071,
    1379.07455417,
    1377.57672104,
    1376.086262,
    1374.59961093,
    1373.1167372,
    1371.63857108,
    1370.16297319,
    1368.69158863,
    1367.22781199,
    1365.76831096,
    1364.31151068,
    1362.8558221,
    1361.40016764,
    1359.94374674,
    1358.48544867,
    1357.02300401,
    1355.55682408,
    1354.08658711,
    1352.60968448,
    1351.1287602,
    1349.64512065,
    1348.15846282,
    1346.66616782,
    1345.16607855,
    1343.65835141,
    1342.14511175,
    1340.63014399,
    1339.09304742,
    1337.52637112,
    1335.95364894,
    1334.38223006,
    1332.81103138,
    1331.24772858,
    1329.69653854,
    1328.15433861,
    1326.61972993,
    1325.08796029,
    1323.55438224,
    1322.02219535,
    1320.49192177,
    1318.96169113,
    1317.432706,
    1315.90255946,
    1314.36452089,
    1312.81422679,
    1311.25332637,
    1309.6868102,
    1308.11747685,
    1306.54344907,
    1304.95907624,
    1303.3602152,
    1301.74390309,
    1300.10775443,
    1298.45263913,
    1296.78007612,
    1295.09234583,
    1293.39019508,
    1291.67421399,
    1289.94735243,
    1288.21191536,
    1286.46623999,
    1284.70920124,
    1282.9442224,
    1281.17476606,
    1279.40212508,
    1277.6261545,
    1275.84825096,
    1274.06887318,
    1272.28710816,
    1270.49928505,
    1268.70111065,
    1266.89434131,
    1265.08021394,
    1263.25626088,
    1261.42223417,
    1259.57976379,
    1257.72640146,
    1255.85583308,
    1253.96578733,
    1252.05619553,
    1250.12216168,
    1248.16184243,
    1246.18013248,
    1244.181028,
    1242.16320398,
    1240.12000691,
    1238.05043141,
    1235.95775879,
    1233.84414395,
    1231.6957699,
    1229.50713194,
    1227.28794078,
    1225.04307789,
    1222.77937855,
    1220.49908075,
    1218.2026686,
    1215.89380054,
    1213.57070931,
    1211.23033824,
    1208.87209207,
    1206.49299239,
    1204.09217185,
    1201.67079388,
    1199.22962621,
    1196.76561776,
    1194.27710631,
    1191.76417554,
    1189.22611967,
    1186.66364254,
    1184.08013788,
    1181.47942354,
    1178.86286818,
    1176.2289322,
    1173.57646846,
    1170.90327413,
    1168.20606748,
    1165.48449396,
    1162.73748258,
    1159.96248104,
    1157.15674466,
    1154.31525823,
    1151.43582179,
    1148.51534501,
    1145.54854552,
    1142.5327673,
    1139.46362641,
    1136.34208875,
    1133.17097716,
    1129.95146939,
    1126.68299937,
    1123.36541991,
    1120.00225004,
    1116.5937351,
    1113.13888039,
    1109.63564865,
    1106.07958083,
    1102.46792812,
    1098.79952731,
    1095.07299489,
    1091.28561738,
    1087.43493899,
    1083.51887658,
    1079.5348956,
    1075.48033446,
    1071.32732558,
    1067.06869205,
    1062.72930984,
    1058.32466592,
    1053.86115015,
    1049.33900677,
    1044.7574558,
    1040.11249238,
    1035.39958451,
    1030.61636236,
    1025.76354374,
    1020.84198402,
    1015.8476865,
    1010.77538298,
    1005.62473733,
    1000.40048663,
    995.104132885,
    989.73470897,
    984.29114096,
    978.773538198,
    973.183389307,
    967.521569932,
    961.78879934,
    955.986355001,
    950.115204937,
    944.176200299,
    938.171477924,
    932.103194002,
    925.973667667,
    919.784231302,
    913.534581861,
    907.225299968,
    900.856497155,
    894.428840483,
    887.943238631,
    881.401389467,
    874.805273935,
    868.157658364,
    861.464050917,
    854.72781196,
    847.950485999,
    841.133597625,
    834.279419612,
    827.389833812,
    820.465519764,
    813.508343419,
    806.518633912,
    799.497169344,
    792.446067886,
    785.365607854,
    778.256970907,
    771.123052361,
    763.967431609,
    756.793081666,
    749.601866199,
    742.398065448,
    735.187327767,
    727.974187436,
    720.760538633,
    713.547585987,
    706.338677913,
    699.135959944,
    691.940482755,
    684.754613364,
    677.577166216,
    670.40730965,
    663.247923726,
    656.100334501,
    648.966895917,
    641.849999549,
    634.749605354,
    627.666781095,
    620.603102944,
    613.559916268,
    606.538438814,
    599.542383908,
    592.576381439,
    585.642653631,
    578.74260529,
    571.878194927,
    565.051380612,
    558.263724021,
    551.516912156,
    544.81201153,
    538.150168265,
    531.532684345,
    524.960582134,
    518.434149438,
    511.954664338,
    505.524413869,
    498.938531257,
    492.737790076,
    486.537048896,
    480.336307715,
    474.135566535,
    468.081244968,
    462.026923401,
    455.972601834,
    449.918280267,
    444.087144368,
    438.256008468,
    432.424872569,
    426.59373667,
    420.986676969,
    415.379617269,
    409.772557569,
    404.165497869,
    398.771247521,
    393.376997172,
    387.982746824,
    382.588496475,
    377.407850654,
    372.227204832,
    367.04655901,
    361.865913188,
    356.901880739,
    351.937848291,
    346.973815842,
    342.009783394,
    337.258138801,
    332.506494208,
    327.754849615,
    323.003205022,
    318.454323491,
    313.90544196,
    309.35656043,
    304.807678899,
    300.464516029,
    296.12135316,
    291.77819029,
    287.435027421,
    283.387015958,
    279.339004494,
    275.290993031,
    271.242981568,
    267.194970105,
    263.146958641,
    259.098947178,
    255.050935715,
    251.417348255,
    247.783760795,
    244.150173335,
    240.516585876,
    236.882998416,
    233.249410956,
    229.615823496,
    225.982236036,
    222.689902822,
    219.397569607,
    216.105236392,
    212.812903178,
    209.520569963,
    206.228236748,
    202.935903534,
    199.957443589,
    196.978983644,
    194.000523698,
    191.022063753,
    188.043603808,
    185.065143863,
    182.086683918,
    179.108223973,
    176.452216715,
    173.796209457,
    171.140202199,
    168.484194942,
    165.828187684,
    163.172180426,
    160.516173168,
    157.86016591,
    155.490160205,
    153.1201545,
    150.750148795,
    148.38014309,
    146.010137384,
    143.640131679,
    141.270125974,
    138.900120269,
    136.775497915,
    134.650875561,
    132.526253208,
    130.401630854,
    128.2770085,
    126.152386146,
    124.027763793,
    122.062860176,
    120.09795656,
    118.133052943,
    116.168149326,
    114.313064442,
    112.457979557,
    110.602894672,
    108.747809787,
    107.049982604,
    105.352155421,
    103.654328238,
    101.956501055,
    100.258673872,
    98.5608466894,
    96.8630195064,
    95.1651923235,
    93.6718636897,
    92.1785350559,
    90.685206422,
    89.1918777882,
    87.6985491544,
    86.2052205206,
    84.7118918868,
    83.218563253,
    81.8922722108,
    80.5659811687,
    79.2396901266,
    77.9133990844,
    76.5871080423,
    75.2608170002,
    73.934525958,
    72.7129223239,
    71.4913186898,
    70.2697150557,
    69.0481114216,
    67.8979621885,
    66.7478129555,
    65.5976637225,
    64.4475144894,
    63.4001803863,
    62.3528462832,
    61.30551218,
    60.2581780769,
    59.2108439738,
    58.1635098707,
    57.1161757675,
    56.0688416644,
    55.1557095851,
    54.2425775059,
    53.3294454266,
    52.4163133473,
    51.5031812681,
    50.5900491888,
    49.6769171095,
    48.7637850303,
    47.9584066843,
    47.1530283384,
    46.3476499925,
    45.5422716466,
    44.7368933007,
    43.9315149548,
    43.1261366088,
    42.4160985878,
    41.7060605668,
    40.9960225458,
    40.2859845248,
    39.5759465038,
    38.8659084828,
    38.1558704618,
    37.4458324408,
    36.825016781,
    36.2042011213,
    35.5833854615,
    34.9625698017,
    34.3417541419,
    33.7209384822,
    33.1001228224,
    32.4793071626,
    31.9492444826,
    31.4191818027,
    30.8891191227,
    30.3590564427,
    29.8289937627,
    29.2989310828,
    28.7688684028,
    28.2388057228,
    27.7916491298,
    27.3444925367,
    26.8973359437,
    26.4501793506,
    26.0030227576,
    25.5558661645,
    25.1087095715,
    24.778410471,
    24.4481113705,
    24.11781227,
    23.7875131695,
    23.457214069,
    23.1269149685,
    22.796615868,
    22.4663167675,
    22.136017667,
    21.8057185665,
    21.475419466,
    21.1451203655,
    20.814821265,
    20.4845221645,
    20.154223064,
    19.8239239635,
    19.5823199452,
    19.340715927,
    19.0991119087,
    18.8575078905,
    18.6159038722,
    18.3742998539,
    18.1326958357,
    17.8910918174,
    17.6494877992,
    17.4078837809,
    17.1662797627,
    16.9246757444,
    16.6830717262,
    16.4414677079,
    16.1998636897,
    16.0215306927,
    15.8431976957,
    15.6648646987,
    15.4865317017,
    15.3081987047,
    15.1298657077,
    14.9515327107,
    14.7731997137,
    14.5948667167,
    14.4165337197,
    14.2382007227,
    14.0598677257,
    13.8815347287,
    13.7032017317,
    13.5248687347,
    13.3465357377,
    13.2033478434,
    13.0601599491,
    12.9169720549,
    12.7737841606,
    12.6305962663,
    12.487408372,
    12.3442204777,
    12.2010325835,
    12.0578446892,
    11.9146567949,
    11.7714689006,
    11.6282810064,
    11.4850931121,
    11.3419052178,
    11.1987173235,
    11.0555294292,
    10.912341535,
    10.7691536407,
    10.6259657464,
    10.4827778521,
    10.3395899578,
    10.1964020636,
    10.0532141693,
    9.91002627501,
    9.76683838073,
    9.62365048645,
    9.48046259217,
    9.33727469789,
    9.19408680361,
    9.05089890934,
    8.90771101506,
    8.76452312078,
    8.6213352265,
    8.47814733222,
    8.33495943794,
    8.19177154366,
    8.04858364938,
    7.90539575511,
    7.76220786083,
    7.61901996655,
    7.47583207227,
    7.33264417799,
    7.18945628371,
    7.04626838943,
    6.90308049515,
    6.75989260088,
    6.6167047066,
    6.47351681232,
    6.33032891804,
    6.18714102376,
    6.04395312948,
    5.9007652352,
    5.75757734092,
    5.61438944664,
    5.47120155237,
    5.32801365809,
    5.18482576381,
    5.04163786953,
    4.89844997525,
    4.75526208097,
    4.61207418669,
    4.46888629241,
    4.32569839814,
    4.18251050386,
    4.03932260958,
    3.8961347153,
    3.75294682102,
    3.60975892674,
    3.46657103246,
    3.32338313818,
    3.18019524391,
    3.03700734963,
    2.89381945535,
    2.75063156107,
    2.60744366679,
    2.46425577251,
    2.32106787823,
    2.17787998395,
    2.03469208967,
    1.8915041954,
    1.74831630112,
    1.60512840684,
    1.46194051256,
    1.31875261828,
    1.175564724,
    1.03237682972,
    0.889188935444,
    0.746001041165,
    0.602813146887,
    0.459625252608,
    0.316437358329,
    0.17324946405,
    0.0300615697713,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0 } )
  ,

  jump_Th( 3.0 )
  , // mV
  tau_Th( 3.0 )
  , // ms
  t_refCa( 99.9 )
  , // ms, sye 1.0, syi 2.0
  act_flag( 1.0 )
  , reset_flag( 1.0 )

{
  // conductances between compartments
  g_conn[ SOMA ] = 2.5; // nS, soma-proximal
  g_conn[ PROX ] = 1.0; // nS, proximal-distal

  // soma parameters
  t_L[ SOMA ] = 500.0;    // nS
  nt_L[ SOMA ] = 15.0;    // nS
  g_L[ SOMA ] = 10.0;     // nS
  C_m[ SOMA ] = 150.0;    // pF
  E_ex[ SOMA ] = 0.0;     // mV
  E_in[ SOMA ] = -85.0;   // mV
  E_L[ SOMA ] = -70.0;    // mV
  tau_synE[ SOMA ] = 0.5; // ms
  tau_synI[ SOMA ] = 2.0; // ms
  I_e[ SOMA ] = 0.0;      // pA
  // changes
  tau_curAP[ SOMA ] = 1.0; // ms
  amp_curAP[ SOMA ] = 0.0; // pA

  // proximal parameters
  t_L[ PROX ] = 5.0;      // nS
  nt_L[ PROX ] = 10.0;    // nS
  g_L[ PROX ] = 5.0;      // nS
  C_m[ PROX ] = 75.0;     // pF
  E_ex[ PROX ] = 0.0;     // mV
  E_in[ PROX ] = -85.0;   // mV
  E_L[ PROX ] = -70.0;    // mV
  tau_synE[ PROX ] = 0.5; // ms
  tau_synI[ PROX ] = 2.0; // ms
  I_e[ PROX ] = 0.0;      // pA
  // changes
  tau_curAP[ PROX ] = 1.0; // ms
  amp_curAP[ PROX ] = 0.0; // pA

  // distal parameters
  t_L[ DIST ] = 5.0;      // nS
  nt_L[ DIST ] = 15.0;    // nS
  g_L[ DIST ] = 10.0;     // nS
  C_m[ DIST ] = 150.0;    // pF
  E_ex[ DIST ] = 0.0;     // mV
  E_in[ DIST ] = -85.0;   // mV
  E_L[ DIST ] = -70.0;    // mV
  tau_synE[ DIST ] = 0.5; // ms
  tau_synI[ DIST ] = 2.0; // ms
  I_e[ DIST ] = 0.0;      // pA
  // changes
  tau_curAP[ DIST ] = 1.0; // ms
  amp_curAP[ DIST ] = 0.0; // pA
}

nest::iaf_cond_alpha_mc_fixedca::Parameters_::Parameters_( const Parameters_& p )
  : V_th( p.V_th )
  , V_reset( p.V_reset )
  , t_ref( p.t_ref )
  ,
  // changes
  V_max( p.V_max )
  , V_thCa( p.V_thCa )
  , amp( p.amp )
  , jump_Th( p.jump_Th )
  , tau_Th( p.tau_Th )
  , t_refCa( p.t_refCa )
  , act_flag( p.act_flag )
  , reset_flag( p.reset_flag )

{
  // copy C-arrays
  for ( size_t n = 0; n < CA_SIZE; ++n )
    Ca_amp[ n ] = p.Ca_amp[ n ];

  for ( size_t n = 0; n < NCOMP - 1; ++n )
    g_conn[ n ] = p.g_conn[ n ];

  for ( size_t n = 0; n < NCOMP; ++n )
  {
    t_L[ n ] = p.t_L[ n ];
    nt_L[ n ] = p.nt_L[ n ];
    g_L[ n ] = p.g_L[ n ];
    C_m[ n ] = p.C_m[ n ];
    E_ex[ n ] = p.E_ex[ n ];
    E_in[ n ] = p.E_in[ n ];
    E_L[ n ] = p.E_L[ n ];
    tau_synE[ n ] = p.tau_synE[ n ];
    tau_synI[ n ] = p.tau_synI[ n ];
    I_e[ n ] = p.I_e[ n ];
    // changes
    tau_curAP[ n ] = p.tau_curAP[ n ];
    amp_curAP[ n ] = p.amp_curAP[ n ];
  }
}

nest::iaf_cond_alpha_mc_fixedca::Parameters_& nest::iaf_cond_alpha_mc_fixedca::Parameters_::
operator=( const Parameters_& p )
{
  assert( this != &p ); // would be bad logical error in program

  V_th = p.V_th;
  V_reset = p.V_reset;
  t_ref = p.t_ref;
  // changes
  V_max = p.V_max;
  V_thCa = p.V_thCa;
  amp = p.amp;
  jump_Th = p.jump_Th;
  tau_Th = p.tau_Th;
  t_refCa = p.t_refCa;
  act_flag = p.act_flag;
  reset_flag = p.reset_flag;

  // copy C-arrays
  for ( size_t n = 0; n < CA_SIZE; ++n )
    Ca_amp[ n ] = p.Ca_amp[ n ];

  // copy C-arrays
  for ( size_t n = 0; n < NCOMP - 1; ++n )
    g_conn[ n ] = p.g_conn[ n ];

  for ( size_t n = 0; n < NCOMP; ++n )
  {
    t_L[ n ] = p.t_L[ n ];
    nt_L[ n ] = p.nt_L[ n ];
    g_L[ n ] = p.g_L[ n ];
    C_m[ n ] = p.C_m[ n ];
    E_ex[ n ] = p.E_ex[ n ];
    E_in[ n ] = p.E_in[ n ];
    E_L[ n ] = p.E_L[ n ];
    tau_synE[ n ] = p.tau_synE[ n ];
    tau_synI[ n ] = p.tau_synI[ n ];
    I_e[ n ] = p.I_e[ n ];
    // changes
    tau_curAP[ n ] = p.tau_curAP[ n ];
    amp_curAP[ n ] = p.amp_curAP[ n ];
  }

  return *this;
}


nest::iaf_cond_alpha_mc_fixedca::State_::State_( const Parameters_& p )
  : r_( 0 )
  ,
  // changes
  rCa_( 0 )
  , numCa_( 0 )
  , th_( p.V_th )
  , ICa_( 0.0 )
// reset_flag_(0)

{
  // for simplicity, we first initialize all values to 0,
  // then set the membrane potentials for each compartment
  for ( size_t i = 0; i < STATE_VEC_SIZE; ++i )
    y_[ i ] = 0;

  y_[ idx( 0, V_M ) ] = -70.;
  y_[ idx( 1, V_M ) ] = -65.;
  y_[ idx( 2, V_M ) ] = -60.;
}

nest::iaf_cond_alpha_mc_fixedca::State_::State_( const State_& s )
  : r_( s.r_ )
  , rCa_( s.rCa_ )
  , numCa_( s.numCa_ )
  ,
  // changes
  th_( s.th_ )
  , ICa_( s.ICa_ )
{
  for ( size_t i = 0; i < STATE_VEC_SIZE; ++i )
    y_[ i ] = s.y_[ i ];
}

nest::iaf_cond_alpha_mc_fixedca::State_& nest::iaf_cond_alpha_mc_fixedca::State_::operator=(
  const State_& s )
{
  assert( this != &s ); // would be bad logical error in program

  for ( size_t i = 0; i < STATE_VEC_SIZE; ++i )
    y_[ i ] = s.y_[ i ];
  r_ = s.r_;
  // changes
  rCa_ = s.rCa_;
  numCa_ = s.numCa_;
  th_ = s.th_;
  ICa_ = s.ICa_;
  return *this;
}

nest::iaf_cond_alpha_mc_fixedca::Buffers_::Buffers_( iaf_cond_alpha_mc_fixedca& n )
  : logger_( n )
  , s_( 0 )
  , c_( 0 )
  , e_( 0 )
{
  // Initialization of the remaining members is deferred to
  // init_buffers_().
}

nest::iaf_cond_alpha_mc_fixedca::Buffers_::Buffers_( const Buffers_&, iaf_cond_alpha_mc_fixedca& n )
  : logger_( n )
  , s_( 0 )
  , c_( 0 )
  , e_( 0 )
{
  // Initialization of the remaining members is deferred to
  // init_buffers_().
}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::iaf_cond_alpha_mc_fixedca::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::V_th, V_th );
  def< double >( d, names::V_reset, V_reset );
  def< double >( d, names::t_ref, t_ref );
  // changes
  def< double >( d, names::V_max, V_max );
  def< double >( d, names::V_thCa, V_thCa );
  def< double >( d, names::amp, amp );
  def< double >( d, names::jump_Th, jump_Th );
  def< double >( d, names::tau_Th, tau_Th );
  def< double >( d, names::t_refCa, t_refCa );
  def< double >( d, names::act_flag, act_flag );
  def< double >( d, names::reset_flag, reset_flag );
  def< double >( d, Name( "g_sp" ), g_conn[ SOMA ] );
  def< double >( d, Name( "g_pd" ), g_conn[ PROX ] );

  // create subdictionaries for per-compartment parameters
  for ( size_t n = 0; n < NCOMP; ++n )
  {
    DictionaryDatum dd = new Dictionary();
    def< double >( dd, names::t_L, t_L[ n ] );
    def< double >( dd, names::nt_L, nt_L[ n ] );
    def< double >( dd, names::g_L, g_L[ n ] );
    def< double >( dd, names::E_L, E_L[ n ] );
    def< double >( dd, names::E_ex, E_ex[ n ] );
    def< double >( dd, names::E_in, E_in[ n ] );
    def< double >( dd, names::C_m, C_m[ n ] );
    def< double >( dd, names::tau_syn_ex, tau_synE[ n ] );
    def< double >( dd, names::tau_syn_in, tau_synI[ n ] );
    def< double >( dd, names::I_e, I_e[ n ] );
    // changes
    def< double >( dd, names::tau_cur_AP, tau_curAP[ n ] );
    def< double >( dd, names::amp_cur_AP, amp_curAP[ n ] );
    ( *d )[ comp_names_[ n ] ] = dd;
  }
}

void
nest::iaf_cond_alpha_mc_fixedca::Parameters_::set( const DictionaryDatum& d )
{
  // allow setting the membrane potential
  updateValue< double >( d, names::V_th, V_th );
  updateValue< double >( d, names::V_reset, V_reset );
  updateValue< double >( d, names::t_ref, t_ref );
  // changes
  updateValue< double >( d, names::V_max, V_max );
  updateValue< double >( d, names::V_thCa, V_thCa );
  updateValue< double >( d, names::amp, amp );
  updateValue< double >( d, names::jump_Th, jump_Th );
  updateValue< double >( d, names::tau_Th, tau_Th );
  updateValue< double >( d, names::t_refCa, t_refCa );
  updateValue< double >( d, names::act_flag, act_flag );
  updateValue< double >( d, names::reset_flag, reset_flag );
  updateValue< double >( d, Name( "g_sp" ), g_conn[ SOMA ] );
  updateValue< double >( d, Name( "g_pd" ), g_conn[ PROX ] );

  // extract from sub-dictionaries
  for ( size_t n = 0; n < NCOMP; ++n )
    if ( d->known( comp_names_[ n ] ) )
    {
      DictionaryDatum dd = getValue< DictionaryDatum >( d, comp_names_[ n ] );

      updateValue< double >( dd, names::t_L, t_L[ n ] );
      updateValue< double >( dd, names::nt_L, nt_L[ n ] );
      updateValue< double >( dd, names::E_L, E_L[ n ] );
      updateValue< double >( dd, names::E_ex, E_ex[ n ] );
      updateValue< double >( dd, names::E_in, E_in[ n ] );
      updateValue< double >( dd, names::C_m, C_m[ n ] );
      updateValue< double >( dd, names::g_L, g_L[ n ] );
      updateValue< double >( dd, names::tau_syn_ex, tau_synE[ n ] );
      updateValue< double >( dd, names::tau_syn_in, tau_synI[ n ] );
      updateValue< double >( dd, names::I_e, I_e[ n ] );
      // changes
      updateValue< double >( dd, names::tau_cur_AP, tau_curAP[ n ] );
      updateValue< double >( dd, names::amp_cur_AP, amp_curAP[ n ] );
    }

  if ( V_reset >= V_th )
    throw BadProperty( "Reset potential must be smaller than threshold." );

  if ( t_ref < 0 )
    throw BadProperty( "Refractory time cannot be negative." );

  if ( tau_Th <= 0 )
    throw BadProperty( "All time constants must be strictly positive." );

  // apply checks compartment-wise
  for ( size_t n = 0; n < NCOMP; ++n )
  {
    if ( C_m[ n ] <= 0 )
      throw BadProperty(
        "Capacitance (" + comp_names_[ n ].toString() + ") must be strictly positive." );

    if ( tau_synE[ n ] <= 0 || tau_synI[ n ] <= 0 || tau_curAP[ n ] <= 0 )
      throw BadProperty(
        "All time constants (" + comp_names_[ n ].toString() + ") must be strictly positive." );
  }
}

void
nest::iaf_cond_alpha_mc_fixedca::State_::get( DictionaryDatum& d ) const
{
  // we assume here that State_::get() always is called after Parameters_::get(),
  // so that the per-compartment dictionaries exist
  for ( size_t n = 0; n < NCOMP; ++n )
  {
    assert( d->known( comp_names_[ n ] ) );
    DictionaryDatum dd = getValue< DictionaryDatum >( d, comp_names_[ n ] );

    def< double >( dd, names::V_m, y_[ idx( n, V_M ) ] ); // Membrane potential
  }
}

void
nest::iaf_cond_alpha_mc_fixedca::State_::set( const DictionaryDatum& d, const Parameters_& )
{
  // extract from sub-dictionaries
  for ( size_t n = 0; n < NCOMP; ++n )
    if ( d->known( comp_names_[ n ] ) )
    {
      DictionaryDatum dd = getValue< DictionaryDatum >( d, comp_names_[ n ] );
      updateValue< double >( dd, names::V_m, y_[ idx( n, V_M ) ] );
    }
}


/* ----------------------------------------------------------------
 * Default and copy constructor for node, and destructor
 * ---------------------------------------------------------------- */

nest::iaf_cond_alpha_mc_fixedca::iaf_cond_alpha_mc_fixedca()
  : Archiving_Node()
  , P_()
  , S_( P_ )
  , B_( *this )
{
  recordablesMap_.create();

  // set up table of compartment names
  // comp_names_.resize(NCOMP); --- Fixed size, see comment on definition
  comp_names_[ SOMA ] = Name( "soma" );
  comp_names_[ PROX ] = Name( "proximal" );
  comp_names_[ DIST ] = Name( "distal" );
}

nest::iaf_cond_alpha_mc_fixedca::iaf_cond_alpha_mc_fixedca( const iaf_cond_alpha_mc_fixedca& n )
  : Archiving_Node( n )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
}

nest::iaf_cond_alpha_mc_fixedca::~iaf_cond_alpha_mc_fixedca()
{
  // GSL structs may not have been allocated, so we need to protect destruction
  if ( B_.s_ )
    gsl_odeiv_step_free( B_.s_ );
  if ( B_.c_ )
    gsl_odeiv_control_free( B_.c_ );
  if ( B_.e_ )
    gsl_odeiv_evolve_free( B_.e_ );
}

/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
nest::iaf_cond_alpha_mc_fixedca::init_state_( const Node& proto )
{
  const iaf_cond_alpha_mc_fixedca& pr = downcast< iaf_cond_alpha_mc_fixedca >( proto );
  S_ = pr.S_;
}

void
nest::iaf_cond_alpha_mc_fixedca::init_buffers_()
{
  B_.spikes_.resize( NUM_SPIKE_RECEPTORS );
  for ( size_t n = 0; n < NUM_SPIKE_RECEPTORS; ++n )
    B_.spikes_[ n ].clear(); // includes resize

  B_.currents_.resize( NUM_CURR_RECEPTORS );
  for ( size_t n = 0; n < NUM_CURR_RECEPTORS; ++n )
    B_.currents_[ n ].clear(); // includes resize

  B_.logger_.reset();
  Archiving_Node::clear_history();

  B_.step_ = Time::get_resolution().get_ms();
  B_.IntegrationStep_ = B_.step_;

  if ( B_.s_ == 0 )
    B_.s_ = gsl_odeiv_step_alloc( gsl_odeiv_step_rkf45, State_::STATE_VEC_SIZE );
  else
    gsl_odeiv_step_reset( B_.s_ );

  if ( B_.c_ == 0 )
    B_.c_ = gsl_odeiv_control_y_new( 1e-3, 0.0 );
  else
    gsl_odeiv_control_init( B_.c_, 1e-3, 0.0, 1.0, 0.0 );

  if ( B_.e_ == 0 )
    B_.e_ = gsl_odeiv_evolve_alloc( State_::STATE_VEC_SIZE );
  else
    gsl_odeiv_evolve_reset( B_.e_ );

  B_.sys_.function = iaf_cond_alpha_mc_fixedca_dynamics;
  B_.sys_.jacobian = NULL;
  B_.sys_.dimension = State_::STATE_VEC_SIZE;
  B_.sys_.params = reinterpret_cast< void* >( this );

  for ( size_t n = 0; n < NCOMP; ++n )
    B_.I_stim_[ n ] = 0.0;
}

void
nest::iaf_cond_alpha_mc_fixedca::calibrate()
{
  B_.logger_.init(); // ensures initialization in case mm connected after Simulate

  for ( size_t n = 0; n < NCOMP; ++n )
  {
    V_.PSConInit_E_[ n ] = 1.0 * numerics::e / P_.tau_synE[ n ];
    V_.PSConInit_I_[ n ] = 1.0 * numerics::e / P_.tau_synI[ n ];
    // changes
    V_.PSConInit_AP_[ n ] = 1.0 * numerics::e / P_.tau_curAP[ n ];
  }

  V_.RefractoryCounts_ = Time( Time::ms( P_.t_ref ) ).get_steps();

  assert( V_.RefractoryCounts_ >= 0 ); // since t_ref >= 0, this can only fail in error

  // changes
  V_.RefractoryCountsCa_ = Time( Time::ms( P_.t_refCa ) ).get_steps();
  assert( V_.RefractoryCountsCa_ >= 0 ); // since t_ref >= 0, this can only fail in error
}


/* ----------------------------------------------------------------
 * Update and spike handling functions
 * ---------------------------------------------------------------- */

void
nest::iaf_cond_alpha_mc_fixedca::update( Time const& origin, const long_t from, const long_t to )
{

  assert( to >= 0 && ( delay ) from < kernel().connection_builder_manager.get_min_delay() );
  assert( from < to );

  for ( long_t lag = from; lag < to; ++lag )
  {

    double t = 0.0;

    // numerical integration with adaptive step size control:
    // ------------------------------------------------------
    // gsl_odeiv_evolve_apply performs only a single numerical
    // integration step, starting from t and bounded by step;
    // the while-loop ensures integration over the whole simulation
    // step (0, step] if more than one integration step is needed due
    // to a small integration step size;
    // note that (t+IntegrationStep > step) leads to integration over
    // (t, step] and afterwards setting t to step, but it does not
    // enforce setting IntegrationStep to step-t; this is of advantage
    // for a consistent and efficient integration across subsequent
    // simulation intervals
    while ( t < B_.step_ )
    {
      const int status = gsl_odeiv_evolve_apply( B_.e_,
        B_.c_,
        B_.s_,
        &B_.sys_,             // system of ODE
        &t,                   // from t
        B_.step_,             // to t <= step
        &B_.IntegrationStep_, // integration step size
        S_.y_ );              // neuronal state

      if ( status != GSL_SUCCESS )
        throw GSLSolverFailure( get_name(), status );
    }

    // changes
    S_.th_ += ( S_.th_ - P_.V_th ) * ( pow( numerics::e, ( -0.1 / P_.tau_Th ) ) - 1.0 );

    // add incoming spikes at end of interval
    // exploit here that spike buffers are compartment for compartment,
    // alternating between excitatory and inhibitory
    for ( size_t n = 0; n < NCOMP; ++n )
    {
      S_.y_[ n * State_::STATE_VEC_COMPS + State_::DG_EXC ] +=
        B_.spikes_[ 2 * n ].get_value( lag ) * V_.PSConInit_E_[ n ];
      S_.y_[ n * State_::STATE_VEC_COMPS + State_::DG_INH ] +=
        B_.spikes_[ 2 * n + 1 ].get_value( lag ) * V_.PSConInit_I_[ n ];
    }

    // refractoriness and spiking
    // exploit here that plain offset enum value V_M indexes soma V_M
    if ( S_.r_ )
    { // neuron is absolute refractory
      --S_.r_;
      if ( S_.r_ == 10 )
      {
        S_.y_[ PROX * State_::STATE_VEC_COMPS + State_::DI_AP ] +=
          P_.amp_curAP[ PROX ] * V_.PSConInit_AP_[ PROX ];
      }
      if ( S_.r_ == 0 )
      {
        if ( P_.reset_flag == 1.0 )
          S_.y_[ State_::V_M ] = P_.V_reset;
        S_.y_[ DIST * State_::STATE_VEC_COMPS + State_::DI_AP ] +=
          P_.amp_curAP[ DIST ] * V_.PSConInit_AP_[ DIST ];
      }
    }
    else if ( S_.y_[ State_::V_M ] >= S_.th_ )
    { // neuron fires spike
      S_.r_ = V_.RefractoryCounts_;
      S_.y_[ State_::V_M ] = P_.V_max;
      S_.th_ += P_.jump_Th;
      P_.g_L[ SOMA ] = P_.t_L[ SOMA ];
      P_.g_L[ PROX ] = P_.t_L[ PROX ];
      P_.g_L[ DIST ] = P_.t_L[ DIST ];
      set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );
      SpikeEvent se;
      kernel().event_delivery_manager.send( *this, se, lag );
    }
    else
    {
      P_.g_L[ SOMA ] = P_.nt_L[ SOMA ];
      P_.g_L[ PROX ] = P_.nt_L[ PROX ];
      P_.g_L[ DIST ] = P_.nt_L[ DIST ];
    }

    if ( S_.rCa_ )
    { // neuron is absolute refractory
      --S_.rCa_;
      if ( S_.rCa_ >= 0 )
      {
        S_.ICa_ = P_.Ca_amp[ V_.RefractoryCountsCa_ - S_.rCa_ ] * P_.amp;
      }
      else
      {
        S_.ICa_ = 0.0;
      }
    }
    else if ( P_.act_flag >= 1.0 )
    { // calcium spike
      if ( ( S_.y_[ DIST * State_::STATE_VEC_COMPS + State_::V_M ] >= P_.V_thCa )
        && ( P_.act_flag == 1.0 ) )
      {
        S_.rCa_ = V_.RefractoryCountsCa_;
        S_.ICa_ = P_.Ca_amp[ 0 ] * P_.amp;
        S_.numCa_ += 1.0;
      }
    }

    // set new input currents
    for ( size_t n = 0; n < NCOMP; ++n )
      B_.I_stim_[ n ] = B_.currents_[ n ].get_value( lag );

    // log state data
    B_.logger_.record_data( origin.get_steps() + lag );
  }
}

void
nest::iaf_cond_alpha_mc_fixedca::handle( SpikeEvent& e )
{
  assert( e.get_delay() > 0 );
  assert( 0 <= e.get_rport() && e.get_rport() < 2 * NCOMP );

  B_.spikes_[ e.get_rport() ].add_value( e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ),
    e.get_weight() * e.get_multiplicity() );
}

void
nest::iaf_cond_alpha_mc_fixedca::handle( CurrentEvent& e )
{
  assert( e.get_delay() > 0 );
  assert( 0 <= e.get_rport() && e.get_rport() < NCOMP ); // not 100% clean, should look at MIN, SUP

  // add weighted current; HEP 2002-10-04
  B_.currents_[ e.get_rport() ].add_value(
    e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), e.get_weight() * e.get_current() );
}

void
nest::iaf_cond_alpha_mc_fixedca::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

#endif // HAVE_GSL
