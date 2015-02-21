import sys

print("#include<stddef.h>")
verts=[]
faces=[]
normals=[]
for l in open(sys.argv[1],'r'):
	lp=l.split()
	if(len(lp)==0):
		pass
	elif(lp[0]=='v'):
		verts.append(lp[1:])
	elif(lp[0]=='vn'):
		normals.append(lp[1:])
	elif(lp[0]=='f'):
		nfi=[]
		for f in lp[1:]:
			fp=f.split('//')
			if(fp[0] != fp[1]):
				raise "NonMatchingIndex!"
			nfi.append(str(int(fp[0])-1))
		faces.append(nfi)

print("const float interleavedverts[6*%d]={\n" % (len(verts)))
for vi in range(len(verts)):
	print(','.join(verts[vi]+normals[vi])+(',' if (vi < len(verts)-1) else ''))
print("};")

print("const size_t indices[3*%d]={\n" % (len(faces)))
for fi in range(len(faces)):
	print(','.join(faces[fi])+(',' if (fi < len(faces)-1) else ''))
print("};")

print("""
#include<string.h>
void gen_bunny_interleaved_array(
	float* out_vertices,
	size_t* out_indices,
	size_t* num_vertices,
	size_t* num_indices)
{
	if(num_indices)
	{
		*num_indices=sizeof(indices)/sizeof(indices[0]);
	}
	if(num_vertices)
	{
		*num_vertices=(sizeof(interleavedverts)/sizeof(interleavedverts[0]));
	}
	if(out_vertices)
	{
		memcpy(out_vertices,interleavedverts,sizeof(interleavedverts));
	}
	if(out_indices)
	{
		memcpy(out_indices,indices,sizeof(indices));
	}
}
""")
