#include<uraster.hpp>
#include<functional>
using namespace std;
///EXAMPLE USAGE

struct TeapotVert
{
	Eigen::Vector3f p;
};

struct TeapotVertVsOut
{
	Eigen::Vector4f p;
	Eigen::Vector3f color;
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
	vout.p=mvp*Eigen::Vector4f(vin.p[0],vin.p[1],vin.p[2],1.0f);
	//vout.p[3]=1.0f;
	vout.color=Eigen::Vector3f(0.0,1.0f,sin(t));
	return vout;
}

TeapotPixel example_fragment_shader(const TeapotVertVsOut& fsin)
{
	TeapotPixel p;
	p.color.head<3>()=fsin.color;
	return p;
}

int main()
{
	const TeapotVert* vbb;
	const TeapotVert* vbe;
	const size_t* ibb;
	const size_t* ibe;
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
	return 0;
}
