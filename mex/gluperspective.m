function [P]=gluperspective(fovy,aspect,near,far)

f=cot((fovy/2.0) * (pi / 180.0));
P=zeros(4);
P(1,1)=f/aspect;
P(2,2)=f;
P(3,3)=(far+near)/(near-far);
P(3,4)=(far*near*2.0)/(near-far);
P(4,3)=-1.0;

end
