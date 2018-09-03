function [P]=glulookat(pos,target,up)

pos=pos(:);
target=target(:);
up=up(:);

f=target-pos;
f=f./norm(f);
up=up./norm(up);

s=cross(f,up);
u=cross(s./norm(s),f);

P=eye(4);
P(1,1:3)=s';
P(2,1:3)=u';
P(3,1:3)=-f';

T=eye(4);
T(1:3,4)=-pos;

P=P*T;

end
