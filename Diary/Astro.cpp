#include <windows.h>
#include "../Headers/general.h"
#include <math.h>
#include "Astro.h"

#define SecsPerDay 24*60*60
#define MSecsPerDay SecsPerDay*1000
typedef double TDateTime;

#define PI 3.14159265
#define DR PI/180

BOOL IsSummerTime(TDate* dat)
{
	if (dat->Month<3 || dat->Month>9) return FALSE;
	if (dat->Month>3 && dat->Month<9) return TRUE;
	TDate d=*dat;
	d.Day=d.Month==3 ? 31 : 30;
	while (TRUE)
	{
		if (DayofWeek(&d)==7) break;
		d--;
	}
	if (d.Month==3)
	{
		if (d.Day>=dat->Day) return TRUE;
		return FALSE;
	}
	else
	{
		if (d.Day>=dat->Day) return FALSE;
		return TRUE;
	}
}

void MakeUlianDate(TDate* dat, double *J, double *F)
{
	double J3=0;
	double D1=dat->Day;

	*F=-0.5;
	*J=-(int)(7*((int)((double)(dat->Month+9)/12)+(double)dat->Year)/4);
	double S;
	int dG=1;
	if (dat->Year<1583) dG=0;

	if (dG)
	{
		S=((dat->Month-9)<0?-1:1);
		double A=abs(dat->Month-9);
		J3=(int)(dat->Year+S*(int)(A/7));
		J3=-(int)(((int)(J3/100)+1)*3/4);
	}

	*J=*J+(int)(275*(double)dat->Month/9)+D1+dG*J3;
	*J=*J+1721027+2*dG+367*dat->Year;
	if (*F<0)
	{
		*F++;
		*J--;
	}
}

void Sunrise(TDate* dat, double Shir, double Dolg, double* t1, double* t2)
{
//Для северного полушария широта является положительной величиной, 
//западные долготы - отрицательные. 
/*
*t1=
	-2 Полярная ночь
	-3 Полярный день
	-4 Сегодня нет восхода
t2=	-4 Сегодня нет заката
*/
	*t1=-1;
	*t2=-1;
	Dolg=Dolg/360;

	TIME_ZONE_INFORMATION tz;
	GetTimeZoneInformation(&tz);
	double Z0=(double)tz.Bias/60;
	if (IsSummerTime(dat)) Z0--;
	Z0=Z0/24;

	double J,F;
	MakeUlianDate(dat,&J,&F);
	double T=(J-2451545)+F;
	double TT=T/36525+1;
	double T0=T/36525;
	double S=24110.5+8640184.813*T0;
	S=S+86636.6*Z0+86400*Dolg;
	S=S/86400;
	double SM=(int)S;
	S=S-SM;

	T0=S*360*DR;
	T=T+Z0;
// funarg 1
	double L=.779072+.00273790931*T;
	double G=.993126+.0027377785*T;
	L=L-(int)L;
	G=G-(int)G;
	L=L*PI*2;
	G=G*PI*2;
	double V=.39785*sin(L);
	V=V-.01000*sin(L-G);
	V=V+.00333*sin(L+G);
	V=V-.00021*TT*sin(L);
	double U=1-.03349*cos(G);
	U=U-.00014*cos(2*L);
	U=U+.00008*cos(L);
	double W=-.00010-.04129*sin(2*L);
	W=W+.03211*sin(G);
	W=W+.00104*sin(2*L-G);
	W=W-.00035*sin(2*L+G);
	W=W-.00008*TT*sin(G);
	S=W/sqrt(U-V*V);
	double AA1=L+atan(S/sqrt(1-S*S));
	S=V/sqrt(U);
	double DA1=atan(S/sqrt(1-S*S));
	double R5=1.00021*sqrt(U);
	T++;

// funarg 2
	L=.779072+.00273790931*T;
	G=.993126+.0027377785*T;
	L=L-(int)L;
	G=G-(int)G;
	L=L*PI*2;
	G=G*PI*2;
	V=.39785*sin(L);
	V=V-.01000*sin(L-G);
	V=V+.00333*sin(L+G);
	V=V-.00021*TT*sin(L);
	U=1-.03349*cos(G);
	U=U-.00014*cos(2*L);
	U=U+.00008*cos(L);
	W=-.00010-.04129*sin(2*L);
	W=W+.03211*sin(G);
	W=W+.00104*sin(2*L-G);
	W=W-.00035*sin(2*L+G);
	W=W-.00008*TT*sin(G);
	S=W/sqrt(U-V*V);
	double AA2=L+atan(S/sqrt(1-S*S));
	S=V/sqrt(U);
	double DA2=atan(S/sqrt(1-S*S));
	R5=1.00021*sqrt(U);
	if (AA2 < AA1) AA2=AA2+PI*2;

	double Z1=DR*(90.833);

	S=sin(Shir*DR);
	double C=cos(Shir*DR);
	double Z=cos(Z1);
	double M8=0;
	double W8=0;
	double A0=AA1;
	double D0=DA1;
	double DA=AA2-AA1;
	double DD=DA2-DA1;
	double V2=0;
	double V0=0;
	for(int C0=0; C0<=23; C0++)
	{
		double P=((double)C0+1)/24;
		double A2=AA1+P*DA;
		double D2=DA1+P*DD;
		double K1=15*DR*1.0027379;
		double L0=T0+C0*K1;
		double L2=L0+K1;
		double H0=L0-A0;
		double H2=L2-A2;
		double H1=(H2+H0)/2;
		double D1=(D2+D0)/2;
		if (C0<=0) V0=S*sin(D0)+C*cos(D0)*cos(H0)-Z;
		V2=S*sin(D2)+C*cos(D2)*cos(H2)-Z;
		
		if ((V0<0?-1:1)==(V2<0?-1:1))
		{
			A0=A2;
			D0=D2;
			V0=V2;
			continue;
		}
		double V1=S*sin(D1)+C*cos(D1)*cos(H1)-Z;
		double A=2.*V2-4.*V1+2.*V0;
		double B=4.*V1-3.*V0-V2;
		double D=B*B-4.*A*V0;
		if (D<0)
		{
			A0=A2;
			D0=D2;
			V0=V2;
			continue;
		}
		D=sqrt(D);
		double EE=(-B+D)/(2*A);
		if (EE>1 || EE<0) EE=(-B-D)/(2*A);
		double T3=C0+EE+(double)1/120;

		double H3=(int)T3;
		double M3=(int)((T3-H3)*60);
		if (V0<0 && V2>0)
		{
			*t1=T3;
			M8=1;
		}
		if (V0>0 && V2<0)
		{
			*t2=T3;
			W8=1;
		}
		double H7=H0+EE*(H2-H0);
		double N7=-cos(D1)*sin(H7);
		double D7=C*sin(D1)-S*cos(D1)*cos(H7);
		double AZ=atan(N7/D7)/DR;
		if (D7<0)
		{
				AZ=AZ+180;
		}
		if (AZ<0)
		{
				AZ=AZ+360;
		}
		if (AZ>360)
		{
				AZ=AZ-360;
		}
		A0=A2;
		D0=D2;
		V0=V2;
	}
	if (M8+W8==0)
	{
		if (V2<0) *t1=-2; //Полярная ночь
		if (V2>0) *t1=-3; //Полярный день
	}
	if (M8+W8!=0)
	{
		if (M8==0) *t1=-4; //Сегодня нет восхода
		if (W8==0) *t2=-4; //Сегодня нет заката
	}
}

void MoonRise(TDate* dat, double Shir, double Dolg, double* t1, double* t2)
{
	TIME_ZONE_INFORMATION tz;
	GetTimeZoneInformation(&tz);
	double Z0=(double)tz.Bias/60;
	if (IsSummerTime(dat)) Z0--;
	Z0=Z0/24;

	Dolg=Dolg/360;

	double J,F;
	MakeUlianDate(dat,&J,&F);

	double T=(J-2451545)+F;
	double T0=T/36525;
	double S=24110.5+8640184.813*T0;
	S=S+86636.6*Z0+86400*Dolg;
	S=S/86400;
	S=S-(int)S;
	T0=S*360*DR;
	T=T+Z0;

	double aM[3][3];

	for (int i=1; i<4; i++)
	{
		double L=0.606434+0.03660110129*T;
		double M=0.374897+0.03629164709*T;
		double F=0.259091+0.03674819520*T;
		double D=0.827362+0.03386319198*T;
		double N=0.347343-0.00014709391*T;
		double G=0.993126+0.00273777850*T;
		L=L-(int)L;  M=M-(int)M;
		F=F-(int)F;  D=D-(int)D;
		N=N-(int)N;  G=G-(int)G;
		L=L*PI*2;		M=M*PI*2;	 F=F*PI*2;
		D=D*PI*2;		N=N*PI*2;	 G=G*PI*2;
		double V=0.39558*sin(F+N);
		V=V+0.08200*sin(F);
		V=V+0.03257*sin(M-F-N);
		V=V+0.01092*sin(M+F+N);
		V=V+0.00666*sin(M-F);
		V=V-0.00644*sin(M+F-2*D+N);
		V=V-0.00331*sin(F-2*D+N);
		V=V-0.00304*sin(F-2*D);
		V=V-0.00240*sin(M-F-2*D-N);
		V=V+0.00226*sin(M+F);
		V=V-0.00108*sin(M+F-2*D);
		V=V-0.00079*sin(F-N);
		V=V+0.00078*sin(F+2*D+N);
		double U=1-0.10828*cos(M);
		U=U-0.01880*cos(M-2*D);
		U=U-0.01479*cos(2*D);
		U=U+0.00181*cos(2*M-2*D);
		U=U-0.00147*cos(2*M);
		U=U-0.00105*cos(2*D-G);
		U=U-0.00075*cos(M-2*D+G);
		double W=0.10478*sin(M);
		W=W-0.04105*sin(2*F+2*N);
		W=W-0.02130*sin(M-2*D);
		W=W-0.01779*sin(2*F+N);
		W=W+0.01774*sin(N);
		W=W+0.00987*sin(2*D);
		W=W-0.00338*sin(M-2*F-2*N);
		W=W-0.00309*sin(G);
		W=W-0.00190*sin(2*F);
		W=W-0.00144*sin(M+N);
		W=W-0.00144*sin(M-2*F-N);
		W=W-0.00113*sin(M+2*F+2*N);
		W=W-0.00094*sin(M-2*D+G);
		W=W-0.00092*sin(2*M-2*D);
		S=W/sqrt(U-V*V);
		double A5=L+atan(S/sqrt(1-S*S));
		S=V/sqrt(U);
		double D5=atan(S/sqrt(1-S*S));
		double R5=60.40974*sqrt(U);
		aM[i-1][0]=A5;
		aM[i-1][1]=D5;
		aM[i-1][2]=R5;
		T=T+0.5;
	}
	if (aM[1][0]<=aM[0][0]) aM[1][0]=aM[1][0]+PI*2;
	if (aM[2][0]<=aM[1][0]) aM[2][0]=aM[2][0]+PI*2;
	double Z1=DR*(90.567-41.685/aM[1][2]);
	S=sin(Shir*DR);
	double C=cos(Shir*DR);
	double Z=cos(Z1);
	double M8=0;
	double W8=0;
	double A0=aM[0][0];
	double D0=aM[0][1];
	double V0=0;
	double V2=0;
	for (int C0=0; C0<24; C0++)
	{
		double P=((double)C0+1)/24;
		double A=aM[1][0]-aM[0][0];
		double B=aM[2][0]-aM[1][0]-A;
		double A2=aM[0][0]+P*(2*A+B*(2*P-1));

		A=aM[1][1]-aM[0][1];
		B=aM[2][1]-aM[1][1]-A;
		double D2=aM[0][1]+P*(2*A+B*(2*P-1));
		double K1=15*DR*1.0027379;
		double L0=T0+C0*K1;
		double L2=L0+K1;
		if (A2<A0) A2=A2+2*PI;
		double H0=L0-A0;
		double H2=L2-A2;
		double H1=(H2+H0)/2;
		double D1=(D2+D0)/2;
		if (C0==0) V0=S*sin(D0)+C*cos(D0)*cos(H0)-Z;
		V2=S*sin(D2)+C*cos(D2)*cos(H2)-Z;
		if ((V0<0?-1:1)==(V2<0?-1:1))
		{
			A0=A2; D0=D2; V0=V2;
			continue;
		}
		double V1=S*sin(D1)+C*cos(D1)*cos(H1)-Z;
		A=2*V2-4*V1+2*V0;
		B=4*V1-3*V0-V2;
		double D=B*B-4*A*V0;
		if (D<0)
		{
			A0=A2;
			D0=D2;
			V0=V2;
			continue;
		}
		D=sqrt(D);
		double E=(-B+D)/(2*A);
		if (E>1 || E<0) E=(-B-D)/(2*A);

		double T3=C0+E+(double)1/120;
		//H3=INT(T3);
		//M3=INT((T3-H3)*60);
		if (V0<0 && V2>0)
		{
			*t1=T3;
			M8=1;
		}
		if (V0>0 && V2<0)
		{
			*t2=T3;
			W8=1;
		}
		double H7=H0+E*(H2-H0);
		double N7=-cos(D1)*sin(H7);
		double D7=C*sin(D1)-S*cos(D1)*cos(H7);
		double A7=atan(N7/D7)/DR;
		if (D7<0) A7=A7+180;
		if (A7<0) A7=A7+360;
		if (A7>360) A7=A7-360;
//435 PRINT USING ",  AZ ###.#";A7
		A0=A2;
		D0=D2;
		V0=V2;
	}
	if (M8+W8==0)
	{
		if (V2<0) *t1=-2; //Полярная ночь
		if (V2>0) *t1=-3; //Полярный день
	}
	if (M8+W8!=0)
	{
		if (M8==0) *t1=-4; //Сегодня нет восхода
		if (W8==0) *t2=-4; //Сегодня нет заката
	}
}

TDateTime EncodeTime(byte Hour, byte Min, byte Sec, byte MSec)
{
    return (Hour * 3600000 + Min * 60000 + Sec * 1000 + MSec) / MSecsPerDay;
}

void CalcPhaseData(TDateTime date, TMoonPhase phase, double *jde, double *kk, double *m, double *ms, double *f, double *o, double *e)
{
	TDate dat;
	dat.Day=1;
	dat.Month=1;
	dat.Year=2000;
  double k=(int)((date-EncodeDate(&dat))/36525.0*1236.85);
  double ts=(date-EncodeDate(&dat))/36525.0;
  *kk=(int)k+phase/4.0;
  double t=*kk/1236.85;
  *jde=2451550.09765+29.530588853**kk
       +t*t*(0.0001337-t*(0.000000150-0.00000000073*t));
  *m=2.5534+29.10535669*(*kk)-t*t*(0.0000218+0.00000011*t);
  *ms=201.5643+385.81693528*(*kk)+t*t*(0.1017438+t*(0.00001239-t*0.000000058));
  *f= 160.7108+390.67050274*(*kk)-t*t*(0.0016341+t*(0.00000227-t*0.000000011));
  *o=124.7746-1.56375580*(*kk)+t*t*(0.0020691+t*0.00000215);
  *e=1-ts*(0.002516+ts*0.0000074);
}

double sin_d(double v)
{
	return sin(v*DR);
}

double cos_d(double v)
{
	return cos(v*DR);
}

TDateTime DelphiDate(double juldat)
{
	TDate dat;
	dat.Day=1;
	dat.Month=1;
	dat.Year=2000;
    return (juldat-2451544.5+EncodeDate(&dat));
}

TDateTime CalcPhase(TDateTime date, TMoonPhase phase)
{
	double korr,kk,jde,m,ms,f,o,e,w;
	CalcPhaseData(date,phase,&jde,&kk,&m,&ms,&f,&o,&e);
	double t=kk/1236.85;
  switch (phase)
  {
  case Newmoon:
  korr= -0.40720*sin_d(ms)
         +0.17241*e*sin_d(m)
         +0.01608*sin_d(2*ms)
         +0.01039*sin_d(2*f)
         +0.00739*e*sin_d(ms-m)
         -0.00514*e*sin_d(ms+m)
         +0.00208*e*e*sin_d(2*m)
         -0.00111*sin_d(ms-2*f)
         -0.00057*sin_d(ms+2*f)
         +0.00056*e*sin_d(2*ms+m)
         -0.00042*sin_d(3*ms)
         +0.00042*e*sin_d(m+2*f)
         +0.00038*e*sin_d(m-2*f)
         -0.00024*e*sin_d(2*ms-m)
         -0.00017*sin_d(o)
         -0.00007*sin_d(ms+2*m)
         +0.00004*sin_d(2*ms-2*f)
         +0.00004*sin_d(3*m)
         +0.00003*sin_d(ms+m-2*f)
         +0.00003*sin_d(2*ms+2*f)
         -0.00003*sin_d(ms+m+2*f)
         +0.00003*sin_d(ms-m+2*f)
         -0.00002*sin_d(ms-m-2*f)
         -0.00002*sin_d(3*ms+m)
         +0.00002*sin_d(4*ms);
		break;
  case FirstQuarter:
  case LastQuarter:
	korr= -0.62801*sin_d(ms)
         +0.17172*e*sin_d(m)
         -0.01183*e*sin_d(ms+m)
         +0.00862*sin_d(2*ms)
         +0.00804*sin_d(2*f)
         +0.00454*e*sin_d(ms-m)
         +0.00204*e*e*sin_d(2*m)
         -0.00180*sin_d(ms-2*f)
         -0.00070*sin_d(ms+2*f)
         -0.00040*sin_d(3*ms)
         -0.00034*e*sin_d(2*ms-m)
         +0.00032*e*sin_d(m+2*f)
         +0.00032*e*sin_d(m-2*f)
         -0.00028*e*e*sin_d(ms+2*m)
         +0.00027*e*sin_d(2*ms+m)
         -0.00017*sin_d(o)
         -0.00005*sin_d(ms-m-2*f)
         +0.00004*sin_d(2*ms+2*f)
         -0.00004*sin_d(ms+m+2*f)
         +0.00004*sin_d(ms-2*m)
         +0.00003*sin_d(ms+m-2*f)
         +0.00003*sin_d(3*m)
         +0.00002*sin_d(2*ms-2*f)
         +0.00002*sin_d(ms-m+2*f)
         -0.00002*sin_d(3*ms+m);
  w=0.00306-0.00038*e*cos_d(m)
            +0.00026*cos_d(ms)
            -0.00002*cos_d(ms-m)
            +0.00002*cos_d(ms+m)
            +0.00002*cos_d(2*f);
  if (phase==FirstQuarter) korr+=w;
  else korr-=w;
  break;
  case Fullmoon:
  korr= -0.40614*sin_d(ms)
         +0.17302*e*sin_d(m)
         +0.01614*sin_d(2*ms)
         +0.01043*sin_d(2*f)
         +0.00734*e*sin_d(ms-m)
         -0.00515*e*sin_d(ms+m)
         +0.00209*e*e*sin_d(2*m)
         -0.00111*sin_d(ms-2*f)
         -0.00057*sin_d(ms+2*f)
         +0.00056*e*sin_d(2*ms+m)
         -0.00042*sin_d(3*ms)
         +0.00042*e*sin_d(m+2*f)
         +0.00038*e*sin_d(m-2*f)
         -0.00024*e*sin_d(2*ms-m)
         -0.00017*sin_d(o)
         -0.00007*sin_d(ms+2*m)
         +0.00004*sin_d(2*ms-2*f)
         +0.00004*sin_d(3*m)
         +0.00003*sin_d(ms+m-2*f)
         +0.00003*sin_d(2*ms+2*f)
         -0.00003*sin_d(ms+m+2*f)
         +0.00003*sin_d(ms-m+2*f)
         -0.00002*sin_d(ms-m-2*f)
         -0.00002*sin_d(3*ms+m)
         +0.00002*sin_d(4*ms);
	break;
  default: korr=0;
}
//   Additional Corrections due to planets
double a[15];
a[1]=299.77+0.107408*kk-0.009173*t*t;
a[2]=251.88+0.016321*kk;
a[3]=251.83+26.651886*kk;
a[4]=349.42+36.412478*kk;
a[5]= 84.66+18.206239*kk;
a[6]=141.74+53.303771*kk;
a[7]=207.14+2.453732*kk;
a[8]=154.84+7.306860*kk;
a[9]= 34.52+27.261239*kk;
a[10]=207.19+0.121824*kk;
a[11]=291.34+1.844379*kk;
a[12]=161.72+24.198154*kk;
a[13]=239.56+25.513099*kk;
a[14]=331.55+3.592518*kk;
double akorr=   +0.000325*sin_d(a[1])
          +0.000165*sin_d(a[2])
          +0.000164*sin_d(a[3])
          +0.000126*sin_d(a[4])
          +0.000110*sin_d(a[5])
          +0.000062*sin_d(a[6])
          +0.000060*sin_d(a[7])
          +0.000056*sin_d(a[8])
          +0.000047*sin_d(a[9])
          +0.000042*sin_d(a[10])
          +0.000040*sin_d(a[11])
          +0.000037*sin_d(a[12])
          +0.000035*sin_d(a[13])
          +0.000023*sin_d(a[14]);
	korr=korr+akorr;
	return DelphiDate(jde+korr);
}

TDateTime LastPhase(TDateTime date, TMoonPhase phase)
{
  TDateTime temp_date=date+28;
  TDateTime res=temp_date;
  while (res>date)
  {
    res=CalcPhase(temp_date,phase);
    temp_date-=28;
  }
  return (res);
}

TDateTime next_phase(TDateTime date, TMoonPhase phase)
{
	TDateTime temp_date=date-28;
	TDateTime res=temp_date;
	while (res<date)
	{
		res=CalcPhase(temp_date,phase);
		temp_date+=28;
    }
	return res;
}

void CalcNearPhase(TDateTime date, TMoonPhase mp, int *m, TDateTime *dp, TMoonPhase* moonp, BOOL bNext)
{
	TDateTime dateph;
	if (bNext) dateph=next_phase(date,mp);
	else dateph=LastPhase(date,mp);
	int mm=(int)(dateph-date);
	if (mm<0) mm=0-mm;
	if (mm<*m)
	{
		*m=mm;
		*dp=dateph;
		*moonp=mp;
	}
}

TMoonPhase MoonPhase(TDate* dat, TDate* datephase, double* Time)
{
	TDateTime date=EncodeDate(dat);
	int m=1000;
	TMoonPhase moonphase;
	TDateTime dp;
	CalcNearPhase(date,Newmoon,&m,&dp,&moonphase,TRUE);
	CalcNearPhase(date,FirstQuarter,&m,&dp,&moonphase,TRUE);
	CalcNearPhase(date,Fullmoon,&m,&dp,&moonphase,TRUE);
	CalcNearPhase(date,LastQuarter,&m,&dp,&moonphase,TRUE);

	CalcNearPhase(date,Newmoon,&m,&dp,&moonphase,FALSE);
	CalcNearPhase(date,FirstQuarter,&m,&dp,&moonphase,FALSE);
	CalcNearPhase(date,Fullmoon,&m,&dp,&moonphase,FALSE);
	CalcNearPhase(date,LastQuarter,&m,&dp,&moonphase,FALSE);

	*datephase=*dat;
	while ((int)dp>(int)date)
	{
		(*datephase)++;
		date++;
	}
	while ((int)dp<(int)date)
	{
		(*datephase)--;
		date--;
	}
	*Time=(dp-(int)dp)*24;
	if (IsSummerTime(datephase))
	{
		(*Time)++;
		if (*Time>23)
		{
			(*Time)-=24;
			(*datephase)++;
		}
	}

	return moonphase;
}