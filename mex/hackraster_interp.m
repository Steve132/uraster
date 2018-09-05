%interpolates 3D attributes according to the underlying barycentric coordinate

function [im]=hackraster_interp(vertmap,barymap,attributes)
	im=zeros([size(vertmap,1),size(vertmap,2),size(attributes,1)]);
	for k=1:size(attributes,1);
		attr=attributes(k,:)';
		attrim=zeros([size(vertmap,1),size(vertmap,2)]);
		for dex=1:3;
			verts=vertmap(:,:,dex);
			barys=barymap(:,:,dex);
			valid=find(verts > 0);
			attrim(valid)=attrim(valid) + attr(verts(valid)).*barys(valid);
		end
		im(:,:,k)=attrim;
	end
end
