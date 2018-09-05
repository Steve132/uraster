%hackraster:

%outputs: 
%	facemap: w x h x 1 int32 image such that each pixel contains the index of the face that it was taken from
%	barymap: w x h x 3 float image such that each pixel contains the barycentric coordinate (0,1) of the linear interpolation of each vertex
%	vertmap: w x h x 3 int32 image such that each pixel contains the 3 vertex ids corresponding to that pixel.
%
%	rasterization can be built from a column of vertex attribute data by doing 

function [barymap,vertmap,facemap]=hackraster(faces,positions,az,el)
	%4) render twice: once with facecolors with a binary mask for each color channel
	%1) find all shared vertices
	%2) duplicated all shared vertices
	%3) every vertex used by a face has a color corresponding to the order it comes from in the face.
	%once with 
	%This allows you to build a mapping of pixel to face id and barycentric coordinate.

	M=size(faces,2);
	N=size(positions,2);
	faceids=[1:N]';
	facecolor=bitand([faceids,bitshift(faceids,-8),bitshift(faceids,-16)],255);  %varargin and varargin{:}?

	trimesh(faces',positions(1,:)',positions(2,:)',positions(3,:)','LineStyle','none','FaceLighting','none','FaceColor','flat','FaceVertexCData',double(facecolor)/255.0);
	facemap=uint32(captureimage(az,el));
    
%	facemap=facemap(:,:,1)+facemap(:,:,2)*0x100+facemap(:,:,3)*0x10000; % matlab no hex??
	facemap=facemap(:,:,1)+facemap(:,:,2)*256+facemap(:,:,3)*256*256;
    
	newpositions=positions(:,faces(:));
	newfaces=reshape([1:size(newpositions,2)],3,M);
    
	newcolors=repmat(eye(3),1,M);
	trimesh(newfaces',newpositions(1,:)',newpositions(2,:)',newpositions(3,:)','LineStyle','None','FaceLighting','none','FaceColor','interp','FaceVertexCData',newcolors');
	barymap=captureimage(az,el);
    barymap=double(barymap)./255.0;

	validids=find(facemap > 0);
	vertmap=zeros([size(facemap,1), size(facemap,2), 3],'uint32');
	validfaces=facemap(validids);
	vertmap(validids)=faces(1,validfaces);
	vertmap(validids+numel(facemap))=faces(2,validfaces);
	vertmap(validids+2*numel(facemap))=faces(3,validfaces);
end

function [cimg]=captureimage(az,el)
    set(gcf, 'Color', 'k', 'Renderer', 'OpenGL'); % TODO: set color 'w' as white k for black
	set(gca, 'Projection', 'perspective');
    axis equal;
    axis off;
    view(az,el);
    cimg = frame2im(getframe(gcf));
  %  imwrite(cimg, 'captureimg.png');
  %  imshow(cimg);
    %pause;
end
