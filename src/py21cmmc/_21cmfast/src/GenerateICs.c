#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
//#include <pthread.h>
#include <omp.h>
#include <complex.h>
#include <fftw3.h>
#include <gsl/gsl_interp.h>
#include <gsl/gsl_integration.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_roots.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_spline.h>
#include <gsl/gsl_errno.h>
#include "21CMMC.h"

void ComputeInitialConditions(struct UserParams user_params, struct CosmoParams cosmo_params, struct InitialConditions boxes) {
    
    /*
     Generates the initial conditions: gaussian random density field (DIM^3) as well as the equal or lower resolution velocity fields, and smoothed density field (HII_DIM^3).
     See INIT_PARAMS.H and ANAL_PARAMS.H to set the appropriate parameters.
     Output is written to ../Boxes
     
     Author: Andrei Mesinger
     Date: 9/29/06
     */
    
    fftwf_plan plan;
    
    unsigned long long ct;
    int n_x, n_y, n_z, i, j, k, ii;
    float k_x, k_y, k_z, k_mag, p, a, b, k_sq;
    double pixel_deltax;
    
    float f_pixel_factor;
    
    gsl_rng * r;
    
    /************  INITIALIZATION **********************/
    
    // Removed all references to threads as 21CMMC is always a single core implementation

    printf("%d\n",cosmo_params.RANDOM_SEED);
    // seed the random number generators
//    r = gsl_rng_alloc(gsl_rng_mt19937);
//    gsl_rng_set(r, cosmo_params.RANDOM_SEED);

/*
    // allocate array for the k-space and real-space boxes
    HIRES_box = (fftwf_complex *) fftwf_malloc(sizeof(fftwf_complex)*user_params.KSPACE_NUM_PIXELS);
    HIRES_box_saved = (fftwf_complex *) fftwf_malloc(sizeof(fftwf_complex)*user_params.KSPACE_NUM_PIXELS);
    
    // now allocate memory for the lower-resolution box
    // use HII_DIM from ANAL_PARAMS
    LOWRES_density = (float *) malloc(sizeof(float)*HII_TOT_NUM_PIXELS);
    LOWRES_vx = (float *) malloc(sizeof(float)*HII_TOT_NUM_PIXELS);
    LOWRES_vy= (float *) malloc(sizeof(float)*HII_TOT_NUM_PIXELS);
    LOWRES_vz = (float *) malloc(sizeof(float)*HII_TOT_NUM_PIXELS);
    
    if(SECOND_ORDER_LPT_CORRECTIONS){
        LOWRES_vx_2LPT = (float *) malloc(sizeof(float)*HII_TOT_NUM_PIXELS);
        LOWRES_vy_2LPT = (float *) malloc(sizeof(float)*HII_TOT_NUM_PIXELS);
        LOWRES_vz_2LPT = (float *) malloc(sizeof(float)*HII_TOT_NUM_PIXELS);
    }
    
    // find factor of HII pixel size / deltax pixel size
    f_pixel_factor = DIM/(float)HII_DIM;
*/
 
    /************  END INITIALIZATION ******************/
    
    /************ CREATE K-SPACE GAUSSIAN RANDOM FIELD ***********/
/*
    for (n_x=0; n_x<DIM; n_x++){
        // convert index to numerical value for this component of the k-mode: k = (2*pi/L) * n
        if (n_x>MIDDLE)
            k_x =(n_x-DIM) * DELTA_K;  // wrap around for FFT convention
        else
            k_x = n_x * DELTA_K;
        
        for (n_y=0; n_y<DIM; n_y++){
            // convert index to numerical value for this component of the k-mode: k = (2*pi/L) * n
            if (n_y>MIDDLE)
                k_y =(n_y-DIM) * DELTA_K;
            else
                k_y = n_y * DELTA_K;
            
            // since physical space field is real, only half contains independent modes
            for (n_z=0; n_z<=MIDDLE; n_z++){
                // convert index to numerical value for this component of the k-mode: k = (2*pi/L) * n
                k_z = n_z * DELTA_K;
                
                // now get the power spectrum; remember, only the magnitude of k counts (due to issotropy)
                // this could be used to speed-up later maybe
                k_mag = sqrt(k_x*k_x + k_y*k_y + k_z*k_z);
                p = power_in_k(k_mag);
                
                // ok, now we can draw the values of the real and imaginary part
                // of our k entry from a Gaussian distribution
                a = gsl_ran_ugaussian(r);
                b = gsl_ran_ugaussian(r);
                HIRES_box[C_INDEX(n_x, n_y, n_z)] = sqrt(VOLUME*p/2.0) * (a + b*I);
            }
        }
    }
*/
    /*****  Adjust the complex conjugate relations for a real array  *****/

//    adj_complex_conj(HIRES_box);
    
    /*** Let's also create a lower-resolution version of the density field  ***/
    
/*    memcpy(HIRES_box_saved, HIRES_box, sizeof(fftwf_complex)*KSPACE_NUM_PIXELS);
    
    if (DIM != HII_DIM)
        filter(HIRES_box, 0, L_FACTOR*BOX_LEN/(HII_DIM+0.0));
    // FFT back to real space
    plan = fftwf_plan_dft_c2r_3d(DIM, DIM, DIM, (fftwf_complex *)HIRES_box, (float *)HIRES_box, FFTW_ESTIMATE);
    fftwf_execute(plan);
    // now sample the filtered box
    for (i=0; i<HII_DIM; i++){
        for (j=0; j<HII_DIM; j++){
            for (k=0; k<HII_DIM; k++){
                LOWRES_density[HII_R_INDEX(i,j,k)] =
                *((float *)HIRES_box + R_FFT_INDEX((unsigned long long)(i*f_pixel_factor+0.5),
                                                   (unsigned long long)(j*f_pixel_factor+0.5),
                                                   (unsigned long long)(k*f_pixel_factor+0.5)))/VOLUME;
            }
        }
    }
*/
 
    /******* PERFORM INVERSE FOURIER TRANSFORM *****************/
    // add the 1/VOLUME factor when converting from k space to real space
/*
    memcpy(HIRES_box, HIRES_box_saved, sizeof(fftwf_complex)*KSPACE_NUM_PIXELS);
    
    for (ct=0; ct<KSPACE_NUM_PIXELS; ct++){
        HIRES_box[ct] /= VOLUME;
    }
    plan = fftwf_plan_dft_c2r_3d(DIM, DIM, DIM, (fftwf_complex *)HIRES_box, (float *)HIRES_box, FFTW_ESTIMATE);
    fftwf_execute(plan);
    fftwf_destroy_plan(plan);
    fftwf_cleanup();
    
    for (i=0; i<DIM; i++){
        for (j=0; j<DIM; j++){
            for (k=0; k<DIM; k++){
                *((float *)HIRES_density + R_FFT_INDEX(i,j,k)) = *((float *)HIRES_box + R_FFT_INDEX(i,j,k));
            }
        }
    }
    
    for(ii=0;ii<3;ii++) {
        
        memcpy(HIRES_box, HIRES_box_saved, sizeof(fftwf_complex)*KSPACE_NUM_PIXELS);
        // Now let's set the velocity field/dD/dt (in comoving Mpc)
        
        for (n_x=0; n_x<DIM; n_x++){
            if (n_x>MIDDLE)
                k_x =(n_x-DIM) * DELTA_K;  // wrap around for FFT convention
            else
                k_x = n_x * DELTA_K;
            
            for (n_y=0; n_y<DIM; n_y++){
                if (n_y>MIDDLE)
                    k_y =(n_y-DIM) * DELTA_K;
                else
                    k_y = n_y * DELTA_K;
                
                for (n_z=0; n_z<=MIDDLE; n_z++){
                    k_z = n_z * DELTA_K;
                    
                    k_sq = k_x*k_x + k_y*k_y + k_z*k_z;
                    
                    // now set the velocities
                    if ((n_x==0) && (n_y==0) && (n_z==0)){ // DC mode
                        HIRES_box[0] = 0;
                    }
                    else{
                        if(ii==0) {
                            HIRES_box[C_INDEX(n_x,n_y,n_z)] *= k_x*I/k_sq/VOLUME;
                        }
                        if(ii==1) {
                            HIRES_box[C_INDEX(n_x,n_y,n_z)] *= k_y*I/k_sq/VOLUME;
                        }
                        if(ii==2) {
                            HIRES_box[C_INDEX(n_x,n_y,n_z)] *= k_z*I/k_sq/VOLUME;
                        }
                        // note the last factor of 1/VOLUME accounts for the scaling in real-space, following the FFT
                    }
                }
            }
        }
        
        if (DIM != HII_DIM)
            filter(HIRES_box, 0, L_FACTOR*BOX_LEN/(HII_DIM+0.0));
        
        plan = fftwf_plan_dft_c2r_3d(DIM, DIM, DIM, (fftwf_complex *)HIRES_box, (float *)HIRES_box, FFTW_ESTIMATE);
        fftwf_execute(plan);
        // now sample to lower res
        // now sample the filtered box
        for (i=0; i<HII_DIM; i++){
            for (j=0; j<HII_DIM; j++){
                for (k=0; k<HII_DIM; k++){
                    if(ii==0) {
                        LOWRES_vx[HII_R_INDEX(i,j,k)] =
                        *((float *)HIRES_box + R_FFT_INDEX((unsigned long long)(i*f_pixel_factor+0.5),
                                                           (unsigned long long)(j*f_pixel_factor+0.5),
                                                           (unsigned long long)(k*f_pixel_factor+0.5)));
                    }
                    if(ii==1) {
                        LOWRES_vy[HII_R_INDEX(i,j,k)] =
                        *((float *)HIRES_box + R_FFT_INDEX((unsigned long long)(i*f_pixel_factor+0.5),
                                                           (unsigned long long)(j*f_pixel_factor+0.5),
                                                           (unsigned long long)(k*f_pixel_factor+0.5)));
                    }
                    if(ii==2) {
                        LOWRES_vz[HII_R_INDEX(i,j,k)] =
                        *((float *)HIRES_box + R_FFT_INDEX((unsigned long long)(i*f_pixel_factor+0.5),
                                                           (unsigned long long)(j*f_pixel_factor+0.5),
                                                           (unsigned long long)(k*f_pixel_factor+0.5)));
                    }
                }
            }
        }
    }
    // write out file
*/
    /* *************************************************** *
     *              BEGIN 2LPT PART                        *
     * *************************************************** */
    
    // Generation of the second order Lagrangian perturbation theory (2LPT) corrections to the ZA
    // reference: Scoccimarro R., 1998, MNRAS, 299, 1097-1118 Appendix D
/*
    // Parameter set in ANAL_PARAMS.H
    if(SECOND_ORDER_LPT_CORRECTIONS){
        // use six supplementary boxes to store the gradients of phi_1 (eq. D13b)
        // Allocating the boxes
#define PHI_INDEX(i, j) ((int) ((i) - (j)) + 3*((j)) - ((int)(j))/2  )
        // ij -> INDEX
        // 00 -> 0
        // 11 -> 3
        // 22 -> 5
        // 10 -> 1
        // 20 -> 2
        // 21 -> 4
        
        fftwf_complex *phi_1[6];
        
        for(i = 0; i < 3; ++i){
            for(j = 0; j <= i; ++j){
                phi_1[PHI_INDEX(i, j)] = (fftwf_complex *) fftwf_malloc(sizeof(fftwf_complex)*KSPACE_NUM_PIXELS);
            }
        }
        
        for(i = 0; i < 3; ++i){
            for(j = 0; j <= i; ++j){
                
                // read in the box
                memcpy(HIRES_box, HIRES_box_saved, sizeof(fftwf_complex)*KSPACE_NUM_PIXELS);
                
                // generate the phi_1 boxes in Fourier transform
                for (n_x=0; n_x<DIM; n_x++){
                    if (n_x>MIDDLE)
                        k_x =(n_x-DIM) * DELTA_K;  // wrap around for FFT convention
                    else
                        k_x = n_x * DELTA_K;
                    
                    for (n_y=0; n_y<DIM; n_y++){
                        if (n_y>MIDDLE)
                            k_y =(n_y-DIM) * DELTA_K;
                        else
                            k_y = n_y * DELTA_K;
                        
                        for (n_z=0; n_z<=MIDDLE; n_z++){
                            k_z = n_z * DELTA_K;
                            
                            k_sq = k_x*k_x + k_y*k_y + k_z*k_z;
                            
                            float k[] = {k_x, k_y, k_z};
                            // now set the velocities
                            if ((n_x==0) && (n_y==0) && (n_z==0)){ // DC mode
                                phi_1[PHI_INDEX(i, j)][0] = 0;
                            }
                            else{
                                phi_1[PHI_INDEX(i, j)][C_INDEX(n_x,n_y,n_z)] = -k[i]*k[j]*HIRES_box[C_INDEX(n_x, n_y, n_z)]/k_sq/VOLUME;
                                // note the last factor of 1/VOLUME accounts for the scaling in real-space, following the FFT
                            }
                        }
                    }
                }
                // Now we can generate the real phi_1[i,j]
                plan = fftwf_plan_dft_c2r_3d(DIM, DIM, DIM, (fftwf_complex *)phi_1[PHI_INDEX(i, j)], (float *)phi_1[PHI_INDEX(i, j)], FFTW_ESTIMATE);
                fftwf_execute(plan);
            }
        }
        
        // Then we will have the laplacian of phi_2 (eq. D13b)
        // After that we have to return in Fourier space and generate the Fourier transform of phi_2
        int m, l;
        for (i=0; i<DIM; i++){
            for (j=0; j<DIM; j++){
                for (k=0; k<DIM; k++){
                    *( (float *)HIRES_box + R_FFT_INDEX((unsigned long long)(i), (unsigned long long)(j), (unsigned long long)(k) )) = 0.0;
                    for(m = 0; m < 3; ++m){
                        for(l = m+1; l < 3; ++l){
                            *((float *)HIRES_box + R_FFT_INDEX((unsigned long long)(i),(unsigned long long)(j),(unsigned long long)(k)) ) += ( *((float *)(phi_1[PHI_INDEX(l, l)]) + R_FFT_INDEX((unsigned long long) (i),(unsigned long long) (j),(unsigned long long) (k)))  ) * (  *((float *)(phi_1[PHI_INDEX(m, m)]) + R_FFT_INDEX((unsigned long long)(i),(unsigned long long)(j),(unsigned long long)(k)))  );
                            *((float *)HIRES_box + R_FFT_INDEX((unsigned long long)(i),(unsigned long long)(j),(unsigned long long)(k)) ) -= ( *((float *)(phi_1[PHI_INDEX(l, m)]) + R_FFT_INDEX((unsigned long long)(i),(unsigned long long) (j),(unsigned long long)(k) ) )  ) * (  *((float *)(phi_1[PHI_INDEX(l, m)]) + R_FFT_INDEX((unsigned long long)(i),(unsigned long long)(j),(unsigned long long)(k) ))  );
                            *((float *)HIRES_box + R_FFT_INDEX((unsigned long long)(i),(unsigned long long)(j),(unsigned long long)(k)) ) /= TOT_NUM_PIXELS;
                        }
                    }
                }
            }
        }
        
        plan = fftwf_plan_dft_r2c_3d(DIM, DIM, DIM, (float *)HIRES_box, (fftwf_complex *)HIRES_box, FFTW_ESTIMATE);
        fftwf_execute(plan);
        
        memcpy(HIRES_box_saved, HIRES_box, sizeof(fftwf_complex)*KSPACE_NUM_PIXELS);
        
        // Now we can store the content of box in a back-up file
        // Then we can generate the gradients of phi_2 (eq. D13b and D9)
*/
        /***** Write out back-up k-box RHS eq. D13b *****/
/*
        // For each component, we generate the velocity field (same as the ZA part)
        
        // Now let's set the velocity field/dD/dt (in comoving Mpc)
        
        // read in the box
        // TODO correct free of phi_1
        
        for(ii=0;ii<3;ii++) {
            
            if(ii>0) {
                memcpy(HIRES_box, HIRES_box_saved, sizeof(fftwf_complex)*KSPACE_NUM_PIXELS);
            }
            // set velocities/dD/dt
            for (n_x=0; n_x<DIM; n_x++){
                if (n_x>MIDDLE)
                    k_x =(n_x-DIM) * DELTA_K;  // wrap around for FFT convention
                else
                    k_x = n_x * DELTA_K;
                
                for (n_y=0; n_y<DIM; n_y++){
                    if (n_y>MIDDLE)
                        k_y =(n_y-DIM) * DELTA_K;
                    else
                        k_y = n_y * DELTA_K;
                    
                    for (n_z=0; n_z<=MIDDLE; n_z++){
                        k_z = n_z * DELTA_K;
                        
                        k_sq = k_x*k_x + k_y*k_y + k_z*k_z;
                        
                        // now set the velocities
                        if ((n_x==0) && (n_y==0) && (n_z==0)){ // DC mode
                            HIRES_box[0] = 0;
                        }
                        else{
                            if(ii==0) {
                                HIRES_box[C_INDEX(n_x,n_y,n_z)] *= k_x*I/k_sq;
                            }
                            if(ii==1) {
                                HIRES_box[C_INDEX(n_x,n_y,n_z)] *= k_y*I/k_sq;
                            }
                            if(ii==2) {
                                HIRES_box[C_INDEX(n_x,n_y,n_z)] *= k_z*I/k_sq;
                            }
                            // note the last factor of 1/VOLUME accounts for the scaling in real-space, following the FFT
                        }
                    }
                }
            }
            
            if (DIM != HII_DIM)
                filter(HIRES_box, 0, L_FACTOR*BOX_LEN/(HII_DIM+0.0));
            
            plan = fftwf_plan_dft_c2r_3d(DIM, DIM, DIM, (fftwf_complex *)HIRES_box, (float *)HIRES_box, FFTW_ESTIMATE);
            fftwf_execute(plan);
            // now sample to lower res
            // now sample the filtered box
            for (i=0; i<HII_DIM; i++){
                for (j=0; j<HII_DIM; j++){
                    for (k=0; k<HII_DIM; k++){
                        if(ii==0) {
                            LOWRES_vx_2LPT[HII_R_INDEX(i,j,k)] =
                            *((float *)HIRES_box + R_FFT_INDEX((unsigned long long)(i*f_pixel_factor+0.5),
                                                               (unsigned long long)(j*f_pixel_factor+0.5),
                                                               (unsigned long long)(k*f_pixel_factor+0.5)));
                        }
                        if(ii==1) {
                            LOWRES_vy_2LPT[HII_R_INDEX(i,j,k)] =
                            *((float *)HIRES_box + R_FFT_INDEX((unsigned long long)(i*f_pixel_factor+0.5),
                                                               (unsigned long long)(j*f_pixel_factor+0.5),
                                                               (unsigned long long)(k*f_pixel_factor+0.5)));
                        }
                        if(ii==2) {
                            LOWRES_vz_2LPT[HII_R_INDEX(i,j,k)] =
                            *((float *)HIRES_box + R_FFT_INDEX((unsigned long long)(i*f_pixel_factor+0.5),
                                                               (unsigned long long)(j*f_pixel_factor+0.5),
                                                               (unsigned long long)(k*f_pixel_factor+0.5)));
                        }
                    }
                }
            }
        }
        // deallocate the supplementary boxes
        for(i = 0; i < 3; ++i){
            for(j = 0; j <= i; ++j){
                fftwf_free(phi_1[PHI_INDEX(i,j)]);
            }
        }
    }
*/
    /* *********************************************** *
     *               END 2LPT PART                     *
     * *********************************************** */
    
    // deallocate
//    fftwf_free(HIRES_box);
//    fftwf_free(HIRES_box_saved);
}

/*****  Adjust the complex conjugate relations for a real array  *****/
/*
void adj_complex_conj(){
    int i, j, k;
    
    // corners
    HIRES_box[C_INDEX(0,0,0)] = 0;
    HIRES_box[C_INDEX(0,0,MIDDLE)] = crealf(HIRES_box[C_INDEX(0,0,MIDDLE)]);
    HIRES_box[C_INDEX(0,MIDDLE,0)] = crealf(HIRES_box[C_INDEX(0,MIDDLE,0)]);
    HIRES_box[C_INDEX(0,MIDDLE,MIDDLE)] = crealf(HIRES_box[C_INDEX(0,MIDDLE,MIDDLE)]);
    HIRES_box[C_INDEX(MIDDLE,0,0)] = crealf(HIRES_box[C_INDEX(MIDDLE,0,0)]);
    HIRES_box[C_INDEX(MIDDLE,0,MIDDLE)] = crealf(HIRES_box[C_INDEX(MIDDLE,0,MIDDLE)]);
    HIRES_box[C_INDEX(MIDDLE,MIDDLE,0)] = crealf(HIRES_box[C_INDEX(MIDDLE,MIDDLE,0)]);
    HIRES_box[C_INDEX(MIDDLE,MIDDLE,MIDDLE)] = crealf(HIRES_box[C_INDEX(MIDDLE,MIDDLE,MIDDLE)]);
    
    // do entire i except corners
    for (i=1; i<MIDDLE; i++){
        // just j corners
        for (j=0; j<=MIDDLE; j+=MIDDLE){
            for (k=0; k<=MIDDLE; k+=MIDDLE){
                HIRES_box[C_INDEX(i,j,k)] = conjf(HIRES_box[C_INDEX(DIM-i,j,k)]);
            }
        }
        
        // all of j
        for (j=1; j<MIDDLE; j++){
            for (k=0; k<=MIDDLE; k+=MIDDLE){
                HIRES_box[C_INDEX(i,j,k)] = conjf(HIRES_box[C_INDEX(DIM-i,DIM-j,k)]);
                HIRES_box[C_INDEX(i,DIM-j,k)] = conjf(HIRES_box[C_INDEX(DIM-i,j,k)]);
            }
        }
    } // end loop over i
    
    // now the i corners
    for (i=0; i<=MIDDLE; i+=MIDDLE){
        for (j=1; j<MIDDLE; j++){
            for (k=0; k<=MIDDLE; k+=MIDDLE){
                HIRES_box[C_INDEX(i,j,k)] = conjf(HIRES_box[C_INDEX(i,DIM-j,k)]);
            }
        }
    } // end loop over remaining j
 
}
*/

