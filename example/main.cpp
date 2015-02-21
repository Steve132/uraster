#include<uraster.hpp>
#include<functional>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include"stb_image_write.h"
using namespace std;
///EXAMPLE USAGE

typedef Eigen::Vector3f TeapotVert;


struct TeapotVertVsOut
{
	Eigen::Vector4f p;
	Eigen::Vector3f color;

	TeapotVertVsOut():
		p(0.0f,0.0f,0.0f,0.0f),color(0.0f,0.0f,0.0f)
	{}
	const Eigen::Vector4f& position() const
	{
		return p;
	}
	TeapotVertVsOut& operator+=(const TeapotVertVsOut& tp)
	{
		p+=tp.p;
		color+=tp.color;
		return *this;
	}
	TeapotVertVsOut& operator*=(const float& f)
	{
		p*=f;color*=f;return *this;
	}
};

class TeapotPixel
{
public:
	Eigen::Vector4f color;
	float& depth() { return color[3]; }
	TeapotPixel():color(0.0f,0.0f,0.0f,-1e10f)
	{}
};

TeapotVertVsOut example_vertex_shader(const TeapotVert& vin,const Eigen::Matrix4f& mvp,float t)
{
	TeapotVertVsOut vout;
	vout.p=mvp*Eigen::Vector4f(vin[0],vin[1],vin[2],1.0f);
	//vout.p[3]=1.0f;
	vout.color=Eigen::Vector3f(1.0f,0.0f,0.0f);
	return vout;
}

TeapotPixel example_fragment_shader(const TeapotVertVsOut& fsin)
{
	TeapotPixel p;
	p.color.head<3>()=fsin.color;
	return p;
}

void write_framebuffer(const uraster::Framebuffer<TeapotPixel>& fb,const std::string& filename)
{
	uint8_t* pixels=new uint8_t[fb.width*fb.height*3];
	std::unique_ptr<uint8_t[]> data(pixels);

	const float* fbdata=&(fb(0,0).color[0]);
	for(size_t i=0;i<fb.width*fb.height;i++)
	{
		for(int c=0;c<3;c++)
		{
			pixels[3*i+c]=std::max(0.0f,std::min(fbdata[4*i+c]*255.0f,255.0f));
		}
	}

	if(0==stbi_write_png(filename.c_str(),fb.width,fb.height,3,pixels,0))
	{
		cout << "Failure to write " << filename << endl;
	}
}

int main()
{
	const TeapotVert triangle[3]={
		{-0.5f,-0.5f,0.0f},
		{ 0.5f,-0.5f,0.0f},
		{ 0.0f,.5f,0.0f}};
	const size_t tid[3]={0,1,2};

	const TeapotVert* vbb=triangle;
	const TeapotVert* vbe=triangle+3;
	const size_t* ibb=tid;
	const size_t* ibe=tid+3;

	Eigen::Matrix4f camera_matrix=Eigen::Matrix4f::Identity();
	float time=0.0;

	uraster::Framebuffer<TeapotPixel> tp(640,480);
	uraster::draw(tp,
		vbb,vbe,
		ibb,ibe,
		(TeapotVertVsOut*)NULL,(TeapotVertVsOut*)NULL,
		std::bind(example_vertex_shader,placeholders::_1,camera_matrix,time),
		example_fragment_shader
	);
	write_framebuffer(tp,"out.png");
	return 0;
}
