/* Copyright (C) 2006 Jean-Marc Valin 
   File: window.c

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   
   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
   
   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
   
   - Neither the name of the Xiph.org Foundation nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.
   
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "misc.h"

#ifdef FIXED_POINT
const spx_word16_t lpc_window[200] = {
1310, 1313, 1321, 1333, 1352, 1375, 1403, 1436,
1475, 1518, 1567, 1621, 1679, 1743, 1811, 1884,
1962, 2044, 2132, 2224, 2320, 2421, 2526, 2636,
2750, 2868, 2990, 3116, 3246, 3380, 3518, 3659,
3804, 3952, 4104, 4259, 4417, 4578, 4742, 4909,
5079, 5251, 5425, 5602, 5781, 5963, 6146, 6331,
6518, 6706, 6896, 7087, 7280, 7473, 7668, 7863,
8059, 8256, 8452, 8650, 8847, 9044, 9241, 9438,
9635, 9831, 10026, 10220, 10414, 10606, 10797, 10987,
11176, 11363, 11548, 11731, 11912, 12091, 12268, 12443,
12615, 12785, 12952, 13116, 13277, 13435, 13590, 13742,
13890, 14035, 14176, 14314, 14448, 14578, 14704, 14826,
14944, 15058, 15168, 15273, 15374, 15470, 15562, 15649,
15732, 15810, 15883, 15951, 16015, 16073, 16127, 16175,
16219, 16257, 16291, 16319, 16342, 16360, 16373, 16381,
16384, 16384, 16384, 16384, 16384, 16384, 16384, 16384,
16384, 16384, 16384, 16384, 16384, 16384, 16384, 16384,
16384, 16384, 16384, 16384, 16384, 16384, 16384, 16384,
16384, 16384, 16384, 16384, 16384, 16384, 16384, 16384,
16384, 16384, 16384, 16384, 16384, 16384, 16384, 16384,
16384, 16384, 16384, 16384, 16384, 16384, 16384, 16384,
16384, 16384, 16384, 16361, 16294, 16183, 16028, 15830,
15588, 15304, 14979, 14613, 14207, 13763, 13282, 12766,
12215, 11631, 11016, 10373, 9702, 9007, 8289, 7551,
6797, 6028, 5251, 4470, 3695, 2943, 2248, 1696
};
#else
const spx_word16_t lpc_window[200] = {
   0.080000, 0.080158, 0.080630, 0.081418, 0.082520, 0.083935, 0.085663, 0.087703,
   0.090052, 0.092710, 0.095674, 0.098943, 0.102514, 0.106385, 0.110553, 0.115015,
   0.119769, 0.124811, 0.130137, 0.135744, 0.141628, 0.147786, 0.154212, 0.160902,
   0.167852, 0.175057, 0.182513, 0.190213, 0.198153, 0.206328, 0.214731, 0.223357,
   0.232200, 0.241254, 0.250513, 0.259970, 0.269619, 0.279453, 0.289466, 0.299651,
   0.310000, 0.320507, 0.331164, 0.341965, 0.352901, 0.363966, 0.375151, 0.386449,
   0.397852, 0.409353, 0.420943, 0.432615, 0.444361, 0.456172, 0.468040, 0.479958,
   0.491917, 0.503909, 0.515925, 0.527959, 0.540000, 0.552041, 0.564075, 0.576091,
   0.588083, 0.600042, 0.611960, 0.623828, 0.635639, 0.647385, 0.659057, 0.670647,
   0.682148, 0.693551, 0.704849, 0.716034, 0.727099, 0.738035, 0.748836, 0.759493,
   0.770000, 0.780349, 0.790534, 0.800547, 0.810381, 0.820030, 0.829487, 0.838746,
   0.847800, 0.856643, 0.865269, 0.873672, 0.881847, 0.889787, 0.897487, 0.904943,
   0.912148, 0.919098, 0.925788, 0.932214, 0.938372, 0.944256, 0.949863, 0.955189,
   0.960231, 0.964985, 0.969447, 0.973615, 0.977486, 0.981057, 0.984326, 0.987290,
   0.989948, 0.992297, 0.994337, 0.996065, 0.997480, 0.998582, 0.999370, 0.999842,
   1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000,
   1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000,
   1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000,
   1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000,
   1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000,
   1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000, 1.000000,
   1.000000, 1.000000, 1.000000, 0.998640, 0.994566, 0.987787, 0.978324, 0.966203,
   0.951458, 0.934131, 0.914270, 0.891931, 0.867179, 0.840084, 0.810723, 0.779182,
   0.745551, 0.709930, 0.672424, 0.633148, 0.592223, 0.549781, 0.505964, 0.460932,
   0.414863, 0.367968, 0.320511, 0.272858, 0.225569, 0.179655, 0.137254, 0.103524
};
#endif
