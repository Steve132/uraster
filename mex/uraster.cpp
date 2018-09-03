/*==========================================================
 * uraster.c - calls uraster to render mesh from MATLAB
 *
 * Performs CPU rasterization.
 *
 * The calling syntax is:
 *
 *		 [images, mask] = uraster(int32(faces), single(positions), single(attributes), camera, rows, columns)
 *
 * int 3xM face matrix - faces
 * real float 3xN vertex matrix - positions
 * float kxN - attributes
 * double 4x4 matrix - camera
 * int - rows of output image resolution
 * int - columns of output image resolution
 * outputs: 
 * rows x columns x k matrix - rasterized images by k attributes
 * int rows x columns - mask
 *
 * This is a MEX-file for MATLAB.
 *
 *========================================================*/
/* compile on commandline: /usr/local/MATLAB/R2012a/bin/mex -I/usr/include/eigen3 -I.. -v CXXFLAGS='$CXXFLAGS -std=c++11 -fopenmp' LDFLAGS='$LDFLAGS -fopenmp' uraster.cpp */
/* test on commandline: /usr/local/MATLAB/R2012a/bin/matlab -nodisplay -nodesktop -r "uraster_demo" */

#include "mex.h"
#include "matrix.h"
#include "uraster_mex.hpp"
#include <vector>

/* The gateway function */
void mexFunction( int nlhs, mxArray *plhs[],
                  int nrhs, const mxArray *prhs[])
{
	int* intFaces;			/* input 3xM face matrix - faces */
	float* positions;	/* input real float 3xN vertex matrix - positions */
	float* attributes;	/* input float kxN - attributes */
	double* camera;		/* input double 4x4 matrix - camera */
	int rows;			/* input int - rows of output image resolution */
	int columns;		/* input int - columns of output image resolution */

	float* images;		/* output float rows x columns x k matrix - rasterized images by k attributes */
	int* mask;			/* output int rows x columns matrix - mask */

	/* Verify correct number of arguments */
	if(nrhs!=6) {
		mexErrMsgIdAndTxt("MyToolbox:uraster:nrhs","Six inputs required.");
	}
	if(nlhs!=2) {
		mexErrMsgIdAndTxt("MyToolbox:uraster:nlhs","Two outputs required.");
	}

	/* Read inputs and verify dimensions */
	size_t mfaces = mxGetN(prhs[0]);
    if(mxGetM(prhs[0])!=3) {
    	mexErrMsgTxt("uraster: Input faces must be 3xM int.");
	}
	if(!mxIsInt32(prhs[0])) {
		mexErrMsgTxt("uraster: Input faces must be of type int32.");
	}
	/* faces = mxGetInt32s(prhs[0]); if newer matlab... */
	intFaces = (int*)mxGetData(prhs[0]);
	std::vector<size_t> faces(intFaces, intFaces + (mfaces * 3));
	
	size_t npositions = mxGetN(prhs[1]);
	size_t npositions_cols = mxGetM(prhs[1]);
    if(!((npositions_cols==2) || (npositions_cols==3) || (npositions_cols==4)) ) {
    	mexErrMsgTxt("uraster: Input positions must be 2xN or 3xN or 4xN float.");
	}
	if(!mxIsSingle(prhs[1])) {
		mexErrMsgTxt("uraster: Input positions must be of type float.");
	}
	/* positions = mxGetSingles(prhs[1]); if newer matlab... */
	positions = (float*)mxGetData(prhs[1]);
	
	mwSize kattributes = mxGetM(prhs[2]);
    if(mxGetN(prhs[2])!=npositions) {
    	mexErrMsgTxt("uraster: Input attributes must be kxN float, where N matches 3xN positions.");
	}
	if(!mxIsSingle(prhs[2])) {
		mexErrMsgTxt("uraster: Input attributes must be of type float.");
	}
	/*attributes = mxGetSingles(prhs[2]); if newer matlab... */
	attributes = (float*)mxGetData(prhs[2]);
	
    if((mxGetM(prhs[3])!=4) || (mxGetN(prhs[3])!=4)) {
    	mexErrMsgTxt("uraster: Input camera must be 4x4 double.");
	}
	if(!mxIsDouble(prhs[3])) {
		mexErrMsgTxt("uraster: Input camera must be of type double.");
	}
	/*camera = mxGetDoubles(prhs[3]); if newer matlab... */
	camera = mxGetPr(prhs[3]);
	
	if(mxIsComplex(prhs[4]) || mxGetNumberOfElements(prhs[4])!=1) {
		mexErrMsgIdAndTxt("MyToolbox:uraster:notScalar","uraster: Input rows must be a scalar.");
	}
	if(mxIsComplex(prhs[5]) || mxGetNumberOfElements(prhs[5])!=1) {
		mexErrMsgIdAndTxt("MyToolbox:uraster:notScalar","uraster: Input columns must be a scalar.");
	}
	rows = mxGetScalar(prhs[4]);
	columns = mxGetScalar(prhs[5]);

	/* Allocate outputs */
	mwSize dims[3] = {rows, columns, kattributes};
	plhs[0] = mxCreateNumericArray(3, dims, mxSINGLE_CLASS, mxREAL);
	/*images = mxGetSingles(plhs[0]);  if newer matlab... */
	images = (float*)mxGetData(plhs[0]);
	
	plhs[1] = mxCreateNumericMatrix(rows, columns, mxINT32_CLASS, mxREAL);
	/*mask = mxGetInt32s(plhs[1]); if newer matlab... */
	mask = (int*)mxGetData(plhs[1]);

	/* create eigen3 4x4 camera matrix */
	Eigen::Matrix<float,4,4> eigenCamera;
	eigenCamera << camera[0], camera[4], camera[8], camera[12];
		camera[1], camera[5], camera[9], camera[13];
		camera[2], camera[6], camera[10], camera[14];
		camera[3], camera[7], camera[11], camera[15];

	/*
	size_t num_faces,std::size_t* faces,
	size_t num_vertices,
	const PFloat* positions,size_t num_pdims,
	const AFloat* attributes,size_t num_attrs,
	const Eigen::Matrix<PFloat,4,4>& camera,
	size_t num_img_rows,
	size_t num_img_cols,
	AFloat* outdata,
	int* outmask*/
	/* Call rasterization routine */
	Mex<float,float>::uraster_cpp(mfaces * 3, faces.data(), npositions, positions, npositions_cols, attributes, kattributes, eigenCamera, rows, columns, images, mask);
}
