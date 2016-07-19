#include "bftVoigt.h"
#include "bftFunctions.h"
#include "bftConstants.h"

namespace bft{

	//****************************************************
	namespace mechanics
    {
        Matrix6 Cel(double E, double nu)
        {
            Matrix6 Cel;
            Cel <<  (1-nu), nu, nu, 0, 0, 0,
                    nu, (1-nu), nu, 0, 0, 0,
                    nu, nu, (1-nu), 0, 0, 0,
                    0, 0, 0, (1-2*nu)/2, 0, 0,
                    0, 0, 0, 0, (1-2*nu)/2, 0,
                    0, 0, 0, 0, 0, (1-2*nu)/2;
            Cel *= E/((1+nu)*(1-2*nu));
            return Cel;
        }
        Matrix6 CelInverse(double E, double nu)
        {
            const double G = E / (2*(1+nu));
            Matrix6 CelInv;
            CelInv <<   1./E,   -nu/E,  -nu/E,  0,      0,      0,
                        -nu/E,  1./E,   -nu/E,  0,      0,      0,
                        -nu/E,  -nu/E,  1./E,   0,      0,      0,
                        0,      0,      0,      1./G,   0,      0,
                        0,      0,      0,      0,      1./G,   0,
                        0,      0,      0,      0,      0,      1./G;
            return CelInv;
        }

        double macauly(double scalar)
        {
            return scalar >= 0 ? scalar : 0.0;
        }
        int heaviside(double scalar)
        {
            return scalar >= 0 ? 1 : 0;
        }
    }
	//****************************************************
    namespace Vgt{

        using namespace Constants;

        const Matrix6& P()
        {
            const static Matrix6 P(P_data);
            return P;
        }

        const Matrix6& PInv()
        {
            const static Matrix6 P(PInv_data);
            return P;
        }


        const Vector6& I()
        {
            const static Vector6 I(I_data);
            return I;
        }
        const Vector6& Ihyd()
        {
            const static Vector6 Ihyd(Ihyd_data);
            return Ihyd;
        }


        const Matrix6& Idev()
        {
            const static Matrix6 Idev(Idev_data);
            return Idev;
        }

        Matrix3d voigtToStrain(const Vector6& voigt)
        {
            Matrix3d strain;
            strain <<       
                    voigt[0],   voigt[3]/2, voigt[4]/2,
                    voigt[3]/2, voigt[1],   voigt[5]/2,
                    voigt[4]/2, voigt[5]/2, voigt[2];
            return strain;
        }

        Matrix3d voigtToStress(const Vector6& voigt)
        {
            Matrix3d stress;
            stress <<       
                    voigt[0],   voigt[3], voigt[4],
                    voigt[3],   voigt[1], voigt[5],
                    voigt[4],   voigt[5], voigt[2];
            return stress;
        }

        Vector6 strainToVoigt(const Matrix3d& strainTensor)
        {
            Vector6 strain;
            strain <<       
                    strainTensor(0,0), strainTensor(1,1), strainTensor(2,2), 
                    2*strainTensor(0,1), 2*strainTensor(0,2), 2*strainTensor(1,2);
            return strain;
        }

        Vector6 stressToVoigt(const Matrix3d& stressTensor)
        {
            Vector6 stress;
            stress <<       
                    stressTensor(0,0), stressTensor(1,1), stressTensor(2,2), 
                    stressTensor(0,1), stressTensor(0,2), stressTensor(1,2);
            return stress;
        }

		Vector3d haighWestergaard(const Vector6& stress)
        {
            Vector3d hw;
            const double J2_ = J2(stress);
            const double J3_ = J3(stress) ;
            hw(0) = I1(stress) / sqrt3;
            hw(1) = sqrt(2 * J2_);

            const double x =  3*(sqrt3/2) * J3_ / (pow(J2_, 3./2));
            if(x<= -1)
                hw(2) = 1./3 * Pi ;
            else if(x>= 1)
                hw(2) = 0;
            else if(x!=x)
                hw(2) = 1./3 * Pi;
            else
                hw(2) = 1./3 * acos(x);
            return hw;
        }

		Vector3d haighWestergaardStrain(const Vector6& strain)
        {
                Vector3d hw;
                const double J2_ = J2strain(strain);

                hw(0) = I1(strain)/ sqrt3;		//will be changed to eM if used
                hw(1) = sqrt(2 * J2_);

				if (hw(1)!=0)
				{
					const double J3_ = J3strain(strain);
					const double x =  3.*(sqrt3/2.) * J3_ / (pow(J2_, 3./2));
					if(x<= -1)
							hw(2) = 1./3 * Constants::Pi;
					else if(x>= 1)
							hw(2) = 0;
					else if(x!=x)
							hw(2) = 1./3 * Constants::Pi;
					else
							hw(2) = 1./3 * acos(x);
				}
				else
					hw(2)=0;

                return hw;
        }

        Vector3d principalStrains(const Vector6& voigtStrain)
        {
            SelfAdjointEigenSolver<Matrix3d> es(voigtToStrain(voigtStrain));
            return es.eigenvalues();
        }

		Vector3d principalStrainsHW(const Vector6& voigtStrain)
        {
				//if you wanna sort your eigenvalues after size
				Vector3d hw = haighWestergaardStrain(voigtStrain);
				Vector3d strainPrinc;
				strainPrinc <<	hw(0)/sqrt3+sqrt2_3*hw(1)*std::cos(hw(2)),
								hw(0)/sqrt3+sqrt2_3*hw(1)*(-std::sin(Constants::Pi/6.-hw(2))),
								hw(0)/sqrt3+sqrt2_3*hw(1)*(-std::sin(Constants::Pi/6.+hw(2)));
				return strainPrinc;
        }

        Vector3d principalStresses(const Vector6& voigtStress)
        {
            SelfAdjointEigenSolver<Matrix3d> es(voigtToStress(voigtStress));
            return es.eigenvalues();
        }

		double normStrain(const Vector6& strain)
		{
			return Vgt::voigtToStrain(strain).norm();
		}

		double Evolneg(const Vector6& strain) 
		{
			Vector3d dEpPrincipal = principalStrains(strain);
                
			return  mechanics::macauly( -dEpPrincipal(0))+
					mechanics::macauly( -dEpPrincipal(1))+
					mechanics::macauly( -dEpPrincipal(2));
		}

        double I1(const Vector6& stress)
        {
            return stress.head(3).sum();
        }

        double I2(const Vector6& stress)
        {
            const Vector6& s = stress;

            return s(0)*s(1) + s(1)*s(2) + s(2)*s(0)
                    -s(3)*s(3) -s(4)*s(4) -s(5)*s(5);
        }

		double I2strain(const Vector6& strain)	//you could also use normal I2, but with epsilon12 instead of 2*epsilon12
		{
				const Vector6& e = strain;

				return e(0)*e(1) + e(1)*e(2) + e(2)*e(0)
                        -e(3)/2.*e(3)/2. - e(4)/2.*e(4)/2. - e(5)/2.*e(5)/2.;
		}

        double I3(const Vector6& stress)
        {
            const Vector6& s = stress;
            return  s(0)*s(1)*s(2)
                    +2*s(3)*s(4)*s(5)
                    -s(0)*s(5)*s(5)
                    -s(1)*s(4)*s(4)
                    -s(2)*s(3)*s(3);
        }

		double I3strain(const Vector6& strain)	//you could also use normal I3, but with epsilon12 instead of 2*epsilon12
		{
			return voigtToStrain(strain).determinant();
		}

        double J2(const Vector6& stress)
        {
            double I1_ = I1(stress);
            double I2_ = I2(stress);
            double res = (1./3)* I1_*I1_ - I2_;
            return res >= 0 ? res : 0.0;
        }

		double J2strain(const Vector6& strain)
		{
				return 1./3.*std::pow(I1(strain),2.) - I2strain(strain);
        }

        double J3(const Vector6& stress)
        {
            double I1_ = I1(stress);
            double I2_ = I2(stress);
            double I3_ = I3(stress);

            return (2./27) * pow(I1_,3)     - (1./3)*I1_*I2_        + I3_;
        }

		double J3strain(const Vector6& strain) //determinant of the deviatoric strain tensor
		{
                return voigtToStrain(Idev()*strain).determinant();
        }

        Vector6 dSigmaMdSigma()
        {
            return 1./3 * I();
        }

        Vector6 dRhodSigma(double rho, const Vector6& stress)
        {

            if(rho <= 1e-14)
                return Vector6::Zero();

            Vector6 s = Idev() * stress;

            return 1./rho  * P() * s;
        }

        Vector6 dThetadSigma(double theta, const Vector6& stress)
        {
            if(theta <= 0 || theta >= Pi/3)
                return Vector6::Zero();

            const double J2_ = J2(stress);
            const double J3_ = J3(stress);

            const double cos2_3theta = std::cos(3*theta) * std::cos(3*theta);
            const double dThetadJ2 = 3*sqrt3/4 * J3_/(std::pow(J2_, 2.5) * std::sqrt(1.0 - cos2_3theta)); 
            const double dThetadJ3 = - sqrt3/2 * 1./ (std::pow(J2_, 1.5) * std::sqrt(1.0 - cos2_3theta));

            if(isNaN(dThetadJ2) || isNaN(dThetadJ3))
                return Vector6::Zero(); 


            Vector6 s_ = Idev() * stress;
            const Matrix3d s = voigtToStress (s_);

            return dThetadJ2*P()*s_ + dThetadJ3*(P() * stressToVoigt(s*s) - 2./3 * J2_ *I());
        }

		double dThetaE_dJ2E(const Vector6& strain)
		{
			const Vector3d hw = haighWestergaardStrain(strain);
			const double theta = hw(2);
		
            if(theta <= 1e-15 || theta >= Pi/3 - 1e-15)
			//if (fabs(std::pow(std::cos(3.*theta),2.)-1.)<=1e-16)//(std::pow(std::cos(3.*theta),2)==1.0 || J2strain(strain)==0)
			{
				return 1e16;
			}
			else
			{
				return 3.*std::sqrt(3.)/4.*J3strain(strain)/( std::pow(J2strain(strain),5./2) *std::sqrt(1.-std::pow(std::cos(3.*theta),2.)));
			}
		}

		double dThetaE_dJ3E(const Vector6& strain)
		{
			const Vector3d hw = haighWestergaardStrain(strain);
			const double& theta = hw(2);

			//if (fabs(std::pow(std::cos(3.*theta),2.)-1.)<=1e-16)// || J2strain(strain)==0)
            if(theta <= 1e-15 || theta >= Pi/3 - 1e-15)
			{
				return -1e16;
			}
			else
			{
				return -std::sqrt(3.)/2.*1./   (std::pow(J2strain(strain),3./2)*std::sqrt(1.-std::pow(std::cos(3.*theta),2.)));
			}
		}
		
		Vector6 dJ2E_dE(const Vector6& strain)
		{
			return PInv() * Idev() * strain;
		}

		Vector6 dJ3E_dE(const Vector6& strain)
		{
			Vector6 e=Idev()*strain;
			return PInv()*strainToVoigt(voigtToStrain(e)*voigtToStrain(e))-2./3.*J2strain(strain)*I();
		}

		Vector6 dThetaE_dE(const Vector6& strain)
        
        {
			return dThetaE_dJ2E(strain)*dJ2E_dE(strain) + dThetaE_dJ3E(strain)*dJ3E_dE(strain);
		}

		Vector3d dDeltaEpvneg_dDeltaEpPrincipals(const Vector6& strain)
		{
			Vector3d dEvdEpPrinc=Vector3d::Zero();
			const Vector3d deltaEpPrinc = principalStrainsHW(strain);

			for(int i = 0; i < dEvdEpPrinc.size(); i++)
				dEvdEpPrinc(i) = -mechanics::heaviside(-deltaEpPrinc(i));

			return dEvdEpPrinc;
		}

		Matrix6 dEp_dE(const Matrix6& CelInv, const Matrix6& Cep)
		{
			return Matrix6::Identity() - CelInv*Cep;
		}
		
		RowVector6d dDeltaEpv_dE(const Matrix6& CelInv, const Matrix6& Cep)
		{
				return I().transpose()*(Matrix6::Identity() - CelInv*Cep);
		}

		Matrix36 dDeltaEpPrincipals_dDeltaEp(const Vector6& dEp) //equations from page 218-219 PhD Thesis David Unteregger
		{
				Vector3d dEpPrinc_dEpvol = Vector3d::Zero();
				Vector3d dEpPrinc_dEprho = Vector3d::Zero();
				Vector3d dEPprinc_dEptheta = Vector3d::Zero();
									
				const double sqrt2_3 =		std::sqrt(2./3.);
				const Vector3d hw =			haighWestergaardStrain(dEp);				
				const double& epsM =		hw(0);
				const double& rhoE =		hw(1);
				const double& thetaE =		hw(2);

				dEpPrinc_dEpvol = 1./3.*I().head(3);
				dEpPrinc_dEprho <<	sqrt2_3*std::cos(hw(2)), 
									sqrt2_3*std::cos(hw(2)-2.*Constants::Pi/3.), 
									sqrt2_3*std::cos(hw(2)+2.*Constants::Pi/3.);

				dEPprinc_dEptheta << -sqrt2_3*hw(1)*std::sin(hw(2)), 
									 -sqrt2_3*hw(1)*std::sin(hw(2)-2.*Constants::Pi/3.), 
									 -sqrt2_3*hw(1)*std::sin(hw(2)+2.*Constants::Pi/3.);

				RowVector6d dEpvol_dEp = RowVector6d::Zero();
				dEpvol_dEp = I().transpose();	
				RowVector6d dEprho_dEp = RowVector6d::Zero();
				RowVector6d dEptheta_dEp = RowVector6d::Zero();

				if (rhoE!=0)
				{
					dEprho_dEp = 1./rhoE * dJ2E_dE(dEp).transpose();
					dEptheta_dEp = (dThetaE_dJ2E(dEp)*dJ2E_dE(dEp).transpose()) + (dThetaE_dJ3E(dEp) *dJ3E_dE(dEp).transpose());
				}
				else
				{
					dEprho_dEp << 1.e16,1.e16,1.e16,1.e16,1.e16,1.e16;//1e16 from Code David (Line 67, D_2_Umatsub_damage3_derivatives)
					dEptheta_dEp << 0.,0.,0.,0.,0.,0.;
				}

				Matrix36 dEpPrincdEpAna;
				dEpPrincdEpAna = (dEpPrinc_dEpvol * dEpvol_dEp) + (dEpPrinc_dEprho * dEprho_dEp) + (dEPprinc_dEptheta * dEptheta_dEp);
				
				return dEpPrincdEpAna;
		}
		
		RowVector6d dDeltaEpvneg_dE(const Vector6& dEp, const Matrix6& CelInv, const Matrix6& Cep)
		{	
				return dDeltaEpvneg_dDeltaEpPrincipals(dEp).transpose()*dDeltaEpPrincipals_dDeltaEp(dEp)*dEp_dE(CelInv, Cep);
		}
}
