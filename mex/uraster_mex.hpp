#include<uraster.hpp>
#include<functional>

template<class PFloat,class AFloat>
struct Mex
{
struct Vert
{
	size_t num_positions;
	const PFloat* position_ptr;
	size_t num_attributes;
	const AFloat* attribute_ptr;
};

class Pixel
{
public:
	int drawn;
	float _depth;
	
	float& depth() {return _depth;}
	Eigen::Matrix<AFloat,1,Eigen::Dynamic> attrs;

	Pixel(size_t num_attr):
		drawn(0),_depth(-1e16f),
		attrs(Eigen::Matrix<AFloat,1,Eigen::Dynamic>::Zero(num_attr))
	{}
};

struct VertVsOut
{
	Eigen::Matrix<PFloat,4,1> p;
	Eigen::Matrix<AFloat,1,Eigen::Dynamic> attrs;

	VertVsOut(size_t num_attr=0):
		p(0.0f,0.0f,0.0f,0.0f),
		attrs(Eigen::Matrix<AFloat,1,Eigen::Dynamic>::Zero(num_attr))
	{}
	const Eigen::Matrix<PFloat,4,1>& position() const
	{
		return p;
	}
	VertVsOut& operator+=(const VertVsOut& tp)
	{
		p+=tp.p;
		attrs+=tp.attrs;
		return *this;
	}
	VertVsOut& operator*=(const float& f)
	{
		p*=f;attrs*=f;return *this;
	}
};


static VertVsOut mex_vertex_shader(const Vert& vin,const Eigen::Matrix<PFloat,4,4>& mvp)
{
	VertVsOut vout(vin.num_attributes);
	Eigen::Matrix<PFloat,4,1> pin(0.0f,0.0f,0.0f,1.0f);
	std::copy(vin.position_ptr,vin.position_ptr+vin.num_positions,pin.data());
	vout.p=mvp*pin;
	std::copy(vin.attribute_ptr,vin.attribute_ptr+vin.num_attributes,vout.attrs.data());
	return vout;
}

static Pixel mex_fragment_shader(const VertVsOut& fsin)
{
	Pixel p(fsin.attrs.cols());
	p.drawn=1;
	p.attrs=fsin.attrs;
	return p;
}

static void uraster_cpp(
	size_t num_indices,std::size_t* indices,
	size_t num_vertices,
	const PFloat* positions,size_t num_pdims,
	const AFloat* attributes,size_t num_attrs,
	const Eigen::Matrix<PFloat,4,4>& camera,
	size_t num_img_rows,
	size_t num_img_cols,
	AFloat* outdata,
	int* outmask
)
{
	std::vector<Mex<PFloat,AFloat>::Vert> mv(num_vertices);
	#pragma omp parallel for
	for(size_t i=0;i<num_vertices;i++)
	{
		Mex<PFloat,AFloat>::Vert mvt;
		mvt.num_positions=num_pdims > 4 ? 4 : num_pdims;
		mvt.num_attributes=num_attrs;
		mvt.position_ptr=positions+i*num_pdims;
		mvt.attribute_ptr=attributes+i*num_attrs;
		mv[i]=mvt;
	}

	//NOTE: because the vertex has a default constructor you can use the null vertex cache here
	//MexVertsVsOut<PFloat,AFloat>* nullmvvo=nullptr;
	VertVsOut* nulmvvo=nullptr;
	uraster::Framebuffer<Pixel> tp(num_img_cols,num_img_rows,Pixel(num_attrs));
	uraster::draw(tp,
		mv.data(),mv.data()+mv.size(),
		indices,indices+num_indices,
		nulmvvo,nulmvvo,
		std::bind(mex_vertex_shader,std::placeholders::_1,camera),
		mex_fragment_shader
	);

	//writeback step
	#pragma omp parallel for
	for(size_t r=0;r < num_img_rows;r++)
	{
		for(size_t c=0;c < num_img_cols;c++)
		{
			const Pixel& px=tp(c,r);
			size_t outoff=c*num_img_rows+r;
			outmask[outoff]=px.drawn;
			size_t imsize=num_img_rows*num_img_cols;
			for(size_t k=0;k < attrs.cols(); k++)
			{
				outdata[k*imgsize+outoff]=px.attrs[k];
			}
		}
	}
} 
};
/*
void uraster_test()
{
	std::vector<float> outdata;
	std::vector<size_t> outmask;

	Mex<float,std::complex<float>>::uraster_cpp(10,nullptr,
		20,
		nullptr,3,
		nullptr,3,
		Eigen::Matrix4f::Identity(),
		300,400,
		nullptr,
		nullptr
	);
}*/

