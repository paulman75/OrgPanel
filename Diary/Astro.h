typedef enum {Newmoon,FirstQuarter,Fullmoon,LastQuarter}
TMoonPhase;

void MoonRise(TDate* dat, double Shir, double Dolg, double* t1, double* t2);
void Sunrise(TDate* dat, double Shir, double Dolg, double* t1, double* t2);
BOOL IsSummerTime(TDate* dat);
TMoonPhase MoonPhase(TDate* dat, TDate* datephase, double* Time);
