#ifndef TIMINGS_H
#define TIMINGS_H
#include "vli/vli_config.h"

#ifdef __CUBLAS__
#include "cuda_runtime_api.h"
#endif

//prototype for removing warning of my compiler
unsigned long getcpuclocks();

#if defined(__i386__)
 
unsigned long getcpuclocks()
{
 unsigned long tsc;
 asm(".byte 0x0f, 0x31" : "=A" (x));
 return( tsc );
}
 
#elif (defined(__amd64__) || defined(__x86_64__))
 
unsigned long getcpuclocks()
{
 unsigned long lo, hi;
 asm( "rdtsc" : "=a" (lo), "=d" (hi) );
 return( lo | (hi << 32) );
}

#endif


class Timer
{
public:
    Timer(std::string name_)
    : val(0), name(name_), freq(CPU_FREQ),nCounter(0)
    { }
    
    ~Timer() { std::cout << name << " " << val << ", nCounter : " << nCounter << std::endl; }
    
    Timer & operator+=(double t)
    {
        val += t;
        return *this;
    }
    
    void begin()
    {
        t0 = getcpuclocks();
    }
    
    void end()
    {
		nCounter += 1;
        unsigned long long t1 = getcpuclocks();
        if (t1 < t0)
            assert(true);
        else
            val += (getcpuclocks()-t0)/freq; // to milliseconds
    }

    double GetTime() const 
    {
	return  val;
    }    
protected:
    double val, t0;
    std::string name;
    unsigned long long freq, nCounter;
};

#ifdef __CUBLAS__
class TimerCuda : public Timer
{
public:
	TimerCuda(std::string name_) : Timer(name_)
	{}
	
	~TimerCuda(){}
	
	void begin()
    {
        cudaEventCreate(&start);
		cudaEventCreate(&stop);
		cudaEventRecord(start, 0);
    }
    
    void end()
    {
        nCounter += 1;
		cudaEventRecord(stop, 0);
		cudaEventSynchronize(stop);
		float elapsedTime;
		cudaEventElapsedTime(&elapsedTime, start, stop); // that's our time!
		cudaEventDestroy(start);
		cudaEventDestroy(stop);
		val = static_cast<double>(elapsedTime)/1000; // because time is in milisecond
    }
	
private:
	cudaEvent_t start, stop;
	
};
#endif
#ifdef _OPENMP
class TimerOMP : public Timer
{
public:
	TimerOMP(std::string name_) : Timer(name_), timer_start(0.0), timer_end(0.0){}

	~TimerOMP(){}
	
	void begin()
	{
		timer_start = omp_get_wtime(); 
	}
	
	void end()
	{
		timer_end = omp_get_wtime();
		val += timer_end - timer_start;
	}
	
private:
	double timer_start, timer_end;
	
};
#endif
#endif
