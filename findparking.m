function findparking(fileName)
    lidarData = readtable(fileName);
    theta = deg2rad(table2array(lidarData(:,1)));
    rho = table2array(lidarData(:,2));
    
    i = find(rho>4);
    rho(i)=[];
    theta(i)=[];
    tf = isoutlier(rho,'quartiles');
    i = find(tf);
    rho(i)=[];
    theta(i)=[];
    polarplot(-theta,rho,'*')
    
    %%
    x = rho.*sin(theta);
    y = rho.*cos(theta);
    i = find(x>0&y>0);
    x = x(i);
    y = y(i);
    
    % code for finding outlier
    % ...
    
    plot(x,y,'*')
    axis equal
    hold on
    plot([0.05,0.05,-0.05,-0.05,0.05],[0.1,-0.1,-0.1,0.1,0.1])
    
    %% with angle
    % % find distance and orientation to wall
    [d,i] = min(x.^2+y.^2);
    orientation = atan(y(i)/x(i)); 
    
    plot([0,x(i)],[0,y(i)])
    p = polyfit(x(i:end),y(i:end),1);
    plot(x,p(1)*x+p(2))
    
    dev = (y-(p(1)*x+p(2))).^2;
    
    i = find(abs(dev)>0.25);
    start = i(1)-1;
    last = i(end)+1;
    
    d = sqrt((x(last)-x(start))^2+(y(last)-y(start))^2-dev(start)^2);
    
    plot([x(start),x(last)],[y(start),y(last)])
    
    
    
    %% 
    
    
    %% parallel
    if abs(orientation) < 0.1
        i = find(x>0 & y>0);
        x = x(i);
        y = y(i);
        tf = isoutlier(x,'grubbs');
        i = find(tf);
        p_1 = i(1)-1;
        p_end = i(end)+1;
        plot([x(p_1),x(p_end)],[y(p_1),y(p_end)])
        d = sqrt((x(p_end)-x(p_1)).^2+(y(p_1)-y(p_end)).^2)
    end
end







