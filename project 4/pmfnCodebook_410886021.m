A = load("pmf_xL.txt");
B = load("pmf_xR.txt");
C = load("pmf_yL.txt");
D = load("pmf_yR.txt");
x = 0:1:255;
figure('Name','PMF_xL','NumberTitle','off');    %作圖
bar(x,A(:,2));
xlabel("xL[n]");
saveas(gcf,"pmf_xL.tif");
figure('Name','PMF_xR','NumberTitle','off');
bar(x,B(:,2));
xlabel("xR[n]");
ylabel("Pr(xR[n])");
saveas(gcf,"pmf_xR.tif");

x = -255:1:255;
figure('Name','PMF_yL','NumberTitle','off');
bar(x,C(:,2));
xlabel("yL[n]");
ylabel("Pr(yL[n])");
saveas(gcf,"pmf_yL.tif");
figure('Name','PMF_yR','NumberTitle','off');
bar(x,D(:,2));
xlabel("yR[n]");
ylabel("Pr(yR[n])");
saveas(gcf,"pmf_yR.tif");
%試機率為0時沒有codebook

x = 1;
y = 1;
for i = 1:256
    if(A(i,2)~=0)
        A1(x,:) = A(i,:);
        x=x+1;
    end
    if(B(i,2)~=0)
        B1(y,:) = B(i,:);
        y = y+1;
    end
end
x = 1;
y = 1;
for i = 1:511
    if(C(i,2)~=0)
        C1(x,:) = C(i,:);
        x=x+1;
    end
    if(D(i,2)~=0)
        D1(y,:) = D(i,:);
        y = y+1;
    end
end
dictXL = huffmandict(A1(:,1),A1(:,2));  %產生codebook
dictXR = huffmandict(B1(:,1),B1(:,2));
dictYL = huffmandict(C1(:,1),C1(:,2));
dictYR = huffmandict(D1(:,1),D1(:,2));
newA = cellfun(@num2str,dictXL,'un',0); %cell轉字串，沒用到
newB = cellfun(@num2str,dictXR,'un',0);
newC = cellfun(@num2str,dictYL,'un',0);
newD = cellfun(@num2str,dictYR,'un',0);

cb1 = fopen("cb_xL.txt","w");
cb2 = fopen("cb_xR.txt","w");
cb3 = fopen("cb_yL.txt","w");
cb4 = fopen("cb_yR.txt","w");
x=1;
y=1;
for i = 1:256   %輸出cb.txt
    if(A(i,2)~=0)
        fprintf(cb1,"%d %.15f ",A(i,1),A(i,2));
        for c = 1:length(dictXL{x,2})
            fprintf(cb1,"%d",dictXL{x,2}(c));
        end
        fprintf(cb1,"\n");
        x = x+1;
    else
        fprintf(cb1,"%d %.15f\n",A(i,1),A(i,2));
    end
    if(B(i,2)~=0)
        fprintf(cb2,"%d %.15f ",B(i,1),B(i,2));
        for c = 1:length(dictXR{y,2})
            fprintf(cb2,"%d",dictXR{y,2}(c));
        end
        fprintf(cb2,"\n");
        y = y+1;
    else
        fprintf(cb2,"%d %.15f\n",B(i,1),B(i,2));
    end
end
x=1;
y=1;
for i = 1:511
    if(C(i,2)~=0)
        fprintf(cb3,"%d %.15f ",C(i,1),C(i,2));
        for c = 1:length(dictYL{x,2})
            fprintf(cb3,"%d",dictYL{x,2}(c));
        end
        fprintf(cb3,"\n");
        x = x+1;
    else
        fprintf(cb3,"%d %.15f\n",C(i,1),C(i,2));
    end
    if(D(i,2)~=0)
        fprintf(cb4,"%d %.15f ",D(i,1),D(i,2));
        for c = 1:length(dictYR{y,2})
            fprintf(cb4,"%d",dictYR{y,2}(c));
        end
        fprintf(cb4,"\n");
        y = y+1;
    else
        fprintf(cb4,"%d %.15f\n",D(i,1),D(i,2));
    end
end