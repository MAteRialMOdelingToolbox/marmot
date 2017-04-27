#pragma once

class NISTParallelizableBackendElement{
    
    private:

        typedef void (*pUmatType)( double[],double[],double[],double&,double&,double&,double&,double[],double[],double&,const double[],const double[],const double[2],const double&,const double&,const double&,const double[],const double[],const char[80],const int&,const int&,const int&,const int&,const double[],const int&,const double[3],const double[9],double&,const double&,const double[9],const double[9],const int&,const int&,const int&,const int&,const int[4],const int&,const int);

        typedef void (*pSimpleUelWithUmatType)( double [], double [], double [], const int &, const double [], const int &, const double [], const double [], const double [], const double [2], const double &, const int& , double &, const int [], const int &, pUmatType, int );

        const int elNumber;
        const double* coordinates;
        double *stateVars;
        const int nStateVars;
        const double* properties;
        const int nProperties;
        const int* intProperties;
        const int nIntProperties;
        const pUmatType umat;
        const int nStateVarsUmat;
        const pSimpleUelWithUmatType uel;

        double *stateVarsTemp;

    public:

        NISTParallelizableBackendElement(int elNumber, 
                                        const double* coordinates,
                                        double* stateVars,
                                        int nStateVars,
                                        //double* stateVarsTemp,
                                        const double* properties,
                                        int nProperties,
                                        const int* intProperties,
                                        int nIntProperties,
                                        pUmatType umat,
                                        int nStateVarsUmat,
                                        pSimpleUelWithUmatType uel);

        ~NISTParallelizableBackendElement();

        void computeYourself(double* Pe, double* Ke, const double* UNew, const double* dU, const double time[], double dTime, double &pNewDT );
        void acceptLastState();
};
