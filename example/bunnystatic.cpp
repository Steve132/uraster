#include<uraster.hpp>
#include<functional>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include"stb_image_write.h"
using namespace std;
///EXAMPLE USAGE

typedef struct { float x,y,z,nx,ny,nz; } BunnyVert;


struct BunnyVertVsOut
{
	Eigen::Vector4f p;
	Eigen::Vector3f color;

	BunnyVertVsOut():
		p(0.0f,0.0f,0.0f,0.0f),color(0.0f,0.0f,0.0f)
	{}
	const Eigen::Vector4f& position() const
	{
		return p;
	}
	BunnyVertVsOut& operator+=(const BunnyVertVsOut& tp)
	{
		p+=tp.p;
		color+=tp.color;
		return *this;
	}
	BunnyVertVsOut& operator*=(const float& f)
	{
		p*=f;color*=f;return *this;
	}
};

class BunnyPixel
{
public:
	Eigen::Vector4f color;
	float& depth() { return color[3]; }
	BunnyPixel():color(0.0f,0.0f,0.0f,-1e10f)
	{}
};

BunnyVertVsOut example_vertex_shader(const BunnyVert& vin,const Eigen::Matrix4f& mvp,float t)
{
	BunnyVertVsOut vout;
	vout.p=mvp*Eigen::Vector4f(vin.x,vin.y,vin.z,1.0f);
	//vout.p[3]=1.0f;
	vout.color=Eigen::Vector3f(vin.nx,vin.ny,vin.nz)*0.5f+Eigen::Vector3f(0.5f,0.5f,0.5f);
	return vout;
}

BunnyPixel example_fragment_shader(const BunnyVertVsOut& fsin)
{
	BunnyPixel p;
	p.color.head<3>()=fsin.color;
	return p;
}

void write_framebuffer(const uraster::Framebuffer<BunnyPixel>& fb,const std::string& filename)
{
	uint8_t* pixels=new uint8_t[fb.width*fb.height*3];
	std::unique_ptr<uint8_t[]> data(pixels);

	const float* fbdata=&(fb(0,0).color[0]);
	#pragma omp parallel for
	for(size_t i=0;i<fb.width*fb.height;i++)
	{
		for(int c=0;c<3;c++)
		{
			pixels[3*i+c]=std::max(0.0f,std::min(fbdata[4*i+c]*255.0f,255.0f));
		}
	}

	std::cerr << "Postprocessing complete.  Writing to file" << endl;
	if(0==stbi_write_png(filename.c_str(),fb.width,fb.height,3,pixels,0))
	{
		cout << "Failure to write " << filename << endl;
	}
}

extern "C"
{
void gen_bunny_interleaved_array(
	float* out_vertices,
	size_t* out_indices,
	size_t* num_vertices,
	size_t* num_indices);
}

int main()
{
	size_t num_bunny_indices;
	size_t num_bunny_vertices;
	gen_bunny_interleaved_array(NULL,NULL,&num_bunny_vertices,&num_bunny_indices);
	std::unique_ptr<size_t[]> bunny_inds(new size_t[num_bunny_indices]);
	std::unique_ptr<float []> bunny_verts_f(new float[6*num_bunny_vertices]);
	gen_bunny_interleaved_array(reinterpret_cast<float*>(bunny_verts_f.get()),bunny_inds.get(),NULL,NULL);
	std::unique_ptr<BunnyVert[]> bunny_verts(new BunnyVert[num_bunny_vertices]);
	
	for(size_t i=0;i<num_bunny_vertices;i++)
	{
		const float* bv=&bunny_verts_f[6*i];
		bunny_verts[i]={bv[0],bv[1],bv[2],bv[3],bv[4],bv[5]};
	}

	const BunnyVert* vbb=bunny_verts.get();
	const BunnyVert* vbe=bunny_verts.get()+num_bunny_vertices;
	const size_t* ibb=bunny_inds.get();
	const size_t* ibe=bunny_inds.get()+num_bunny_indices;

	Eigen::Matrix4f model=Eigen::Matrix4f::Identity();
	
	model(1,1)=-1.0f;
	model(0,0)*=0.5;
	model(1,1)*=0.5;
	model(2,2)*=0.5;
	model(0,3)=0.2;
	model(1,3)=0.5;


	Eigen::Matrix4f camera_matrix=model;
	
	float time=0.0;

	uraster::Framebuffer<BunnyPixel> tp(640<<2,480<<2);
	uraster::draw(tp,
		vbb,vbe,
		ibb,ibe,
		(BunnyVertVsOut*)NULL,(BunnyVertVsOut*)NULL,
		std::bind(example_vertex_shader,placeholders::_1,camera_matrix,time),
		example_fragment_shader
	);
	std::cerr << "Rendering complete.  Postprocessing." << endl;
	write_framebuffer(tp,"out.png");
	return 0;
}
