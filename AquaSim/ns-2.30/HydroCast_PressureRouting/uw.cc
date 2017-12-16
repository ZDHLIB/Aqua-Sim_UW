// Luiz May 2009
//  from u/w ns2 code by Al Harris
// Uichin Lee, uclee@cs.ucla.edu Nov.18, 2008


#include "uw.h"

// d: distance in meters
// freq: frequency in kHz
double UnderwaterChannelMetrics::Pathloss(double d, double freq)
{
  double k = 1.5; //2.0;		// free field

   return k * 10.0 * log10(d) + d * thorp(freq);
}
 
//thorp's attentuation function (dB/km)
//in: f in kHz
//out: dB/m (dB re uPa)
double UnderwaterChannelMetrics::thorp(double frequency)
{
    double atten;
    double f;
    f = pow(frequency,2);
    if(f > 0.4)
        atten = 0.11 * f / (1 + f) +
                44 * (f / (4100 + frequency)) +
                2.75 * pow(10,(-4)) * f +
                0.003;
    else
        atten = 0.002 +
                0.11 * (f / (1 + f)) +
                0.011 * f;

    return atten/1000;
}


//estimate the noise
// in: f in kHz
// out: noise  in dB re uPa

// now sure whether this is correct or not...
#define NOISE_BW 2.35
double UnderwaterChannelMetrics::noise(double f)
{

	//return 50-18*log10(f);

    double s; // must be in [0,1]
    double w; // wind speed in m/s (should be positive)
    double turbulence;
    double ship;
    double wind;
    double thermal;
    double n;

    //XXX need to TCL bind s and w
    s = .5;
    w = 0;

    if(s < 0)
    {
        s = 0;
        fprintf(stderr,
                "shipping factor must be positive, it is set to zero\n");
    }
    if(s > 1)
    {
        s = 1;
        fprintf(stderr,
                "shipping factor must be <= 1, it is set to one\n");
    }
    if(w < 0)
    {
        w = 0;
        fprintf(stderr,
                "wind speed must be positive, it is set to zero\n");
    }

    turbulence = 17 - 30 * log10(f);
    turbulence = pow(10.0,(turbulence * 0.1));
    ship = 40 +
            20 * (s - 0.5) +
            26 * log10(f) -
            60 * log10(f + 0.03);
    ship = pow(10.0,(ship * 0.1));
    wind = 50 +
            7.5 * pow(w,0.5) + 20 * log10(f) -
            40 * log10(f + 0.4);
    wind = pow(10.0,(wind * 0.1));
    thermal = -15 + 20 * log10(f);
    thermal = pow(10.0,(thermal * 0.1));
    n = 10 * log10(turbulence + ship + wind + thermal);

    return NOISE_BW*n;
}

// d: distance
// psize: packet size in bytes
double UnderwaterChannelMetrics::PER(double d, int psize)
{

// Tx power and Freq option
//#define Pt 68 // 1.2kW (gosh!)
//#define Pt 50 // 120W
//#define Pt 46.98 // WHOI modem: 50Wa
//#define Pt (6+167) // dB re 1 uPa; UWM2000: 4W ==> 6dBW
#define Pt (0+167) // dB re 1 uPa; UWM1000: 1W ==> 0dBW
#define FREQ 100.0 // Dario's paper: 100khz

	double PL = Pathloss(d, FREQ);
	double Nf = noise(FREQ);
	double SNRdb = Pt - PL - Nf;
	double SNR = pow(10, SNRdb/10);
		
	// BPSK bit error rate w/ Rayleigh fading
	double BER = 0.5*(1-sqrt(SNR/(1+SNR)));
   
   //Aqui está sendo retornado a probabilidade de erro de um pacote de 
   //psize bytes a uma distância d. Eq.(6) do paper pressure routing...
	return 1.0-pow(1.0-BER, 8*psize);
//   return pow(1.0-BER, 8*psize);
}

//Average number of transmissions
double UnderwaterChannelMetrics::NTX(double PER){
   return 1.0/(1.0 - PER);
}

//Channel Utilization Efficiency
double UnderwaterChannelMetrics::utilization(double rate, double payloadSize, double NTX, double RTT){
   int Header = 32;
   
   double tx = (payloadSize + Header)/rate;
   tx += (1500)/rate;//ACK
   tx += RTT;
	
   return payloadSize/ (rate * NTX * tx);
}

