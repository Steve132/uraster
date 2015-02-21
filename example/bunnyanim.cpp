#include<uraster.hpp>
#include<functional>
#include<chrono>
#define cimg_use_xshm
#include "CImg.h"
using namespace cimg_library;
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

class BunnyDisplay
{
public:
	CImg<uint8_t> backing;
	CImgDisplay window;
public:
	BunnyDisplay(size_t width,size_t height,const std::string& title="Rendering the bunny"):
		backing(width,height,1,3),
		window(width,height,title.c_str(),0)
	{}

	void render_framebuffer(const uraster::Framebuffer<BunnyPixel>& fb)
	{
		const float* fbdata=&(fb(0,0).color[0]);
		uint8_t* pixels=backing.data();
		#pragma omp parallel for
		for(size_t y=0;y<fb.height;y++)
		for(size_t x=0;x<fb.width;x++)
		{
			const float* fbp=fbdata+4*(y*fb.width+x);
			for(int c=0;c<3;c++)
			{
				backing(x,y,c)=std::max(0.0f,std::min(fbp[c]*255.0f,255.0f));
			}
		}

		//std::cerr << "Postprocessing complete.  Writing to file" << endl;
		window.display(backing);
	}
	void wait()
	{
		window.wait();
	}
};

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
	std::unique_ptr<BunnyVertVsOut[]> bunny_vc(new BunnyVertVsOut[num_bunny_vertices]);
	
	for(size_t i=0;i<num_bunny_vertices;i++)
	{
		const float* bv=&bunny_verts_f[6*i];
		bunny_verts[i]={bv[0],bv[1],bv[2],bv[3],bv[4],bv[5]};
	}

	const BunnyVert* vbb=bunny_verts.get();
	const BunnyVert* vbe=bunny_verts.get()+num_bunny_vertices;
	const size_t* ibb=bunny_inds.get();
	const size_t* ibe=bunny_inds.get()+num_bunny_indices;

	BunnyVertVsOut* vcb=bunny_vc.get();
	BunnyVertVsOut* vce=bunny_vc.get()+num_bunny_vertices;

	Eigen::Matrix4f model=Eigen::Matrix4f::Identity();
	
	model(1,1)=-1.0f;
	model(0,0)*=0.5;
	model(1,1)*=0.5;
	model(2,2)*=0.5;
	model(0,3)=0.2;
	model(1,3)=0.5;



	
	float time=0.0;

	uraster::Framebuffer<BunnyPixel> tp(640,480);
	BunnyDisplay disp(tp.width,tp.height);

        auto start = std::chrono::high_resolution_clock::now();
	std::size_t numframes=0;
	for(	std::chrono::duration<float> nowtime(0.0);
		nowtime.count() < 36.0;
		nowtime=std::chrono::high_resolution_clock::now()-start)
	{
		Eigen::Matrix4f view=Eigen::Matrix4f::Zero();
		float s=sin(nowtime.count());
		float c=cos(nowtime.count());
		view(0,0)=view(2,2)=c;
		view(0,2)=s;
		view(2,0)=-s;
		view(3,3)=view(1,1)=1.0f;

		Eigen::Matrix4f camera_matrix=view*model;
		uraster::draw(tp,
			vbb,vbe,
			ibb,ibe,
			vcb,vce,
			std::bind(example_vertex_shader,placeholders::_1,camera_matrix,nowtime.count()),
			example_fragment_shader
		);

		numframes++;
		disp.window.set_title("Rendering %f",static_cast<float>(numframes)/nowtime.count());
		disp.render_framebuffer(tp);
		tp.clear();

	}
        auto end = std::chrono::high_resolution_clock::now();
	std::cout << static_cast<float>(numframes) / (end-start).count() << " fps.";
	return 0;
}
