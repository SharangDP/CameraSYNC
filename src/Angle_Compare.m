
for j=1:13
        s='Test%d';
        str=sprintf(s,j);
        ext1='\POS_IDS.txt';
        ext2='\POS_O3M.txt';
        file_ids=strcat(str,ext1);
        file_o3m=strcat(str,ext2);
        fp=fopen(file_ids,'r');
        data_size=[5 Inf];
        data_ids=fscanf(fp,'%d %d %d %d',data_size);
        fclose(fp);
        
%Computation for IDS file%
        
        X1_ids=data_ids(1,:);
        Y1_ids=data_ids(2,:);
        X2_ids=data_ids(3,:);
        Y2_ids=data_ids(4,:);
        Y_ids=Y2_ids-Y1_ids;
        X_ids=X2_ids-X1_ids;
        theta_ids=atan2(Y_ids,X_ids);
        theta_ids=theta_ids*180/pi;

        

%Computation for O3M file%
        fp=fopen(file_o3m,'r');
        data_size=[5 Inf];
        data_o3m=fscanf(fp,'%d %d %d %d',data_size);
        fclose(fp);
        
        X1_o3m=data_o3m(1,:);
        Y1_o3m=data_o3m(2,:);
        X2_o3m=data_o3m(3,:);
        Y2_o3m=data_o3m(4,:);
        
        %Correction for O3M measurements
        fx = 53.91;
        fy = 39.73;
        mx = 32;
        my = 8;
        k1 = 0.7233;
        k2 = -1.253;
        k5 = 1.872;
        for i=1:numel(X1_o3m)
            vd1(:,i) = ( (Y1_o3m(:,i)+0.5) - my ) / fy;
            ud1(:,i) = ( (X1_o3m(:,i)+0.5) - mx ) / fx;
            rd2_1(:,i) = (ud1(:,i)*ud1(:,i)) + (vd1(:,i)*vd1(:,i));
            radial_1(:,i) = 1.0 + (rd2_1(:,i)*(k1 + (rd2_1(:,i)*(k2 + (rd2_1(:,i)*k5)))));
            xcorr_1(:,i) = (ud1(:,i)*radial_1(:,i));
            ycorr_1(:,i) = (vd1(:,i)*radial_1(:,i));
        end
        for i=1:numel(X2_o3m)
            vd2(:,i) = ( (Y2_o3m(:,i)+0.5) - my ) / fy;
            ud2(:,i) = ( (X2_o3m(:,i)+0.5) - mx ) / fx;
            rd2_2(:,i) = (ud2(:,i)*ud2(:,i)) + (vd2(:,i)*vd2(:,i));
            radial_2(:,i) = 1.0 + (rd2_2(:,i)*(k1 + (rd2_2(:,i)*(k2 + (rd2_1(:,i)*k5)))));
            xcorr_2(:,i) = (ud2(:,i)*radial_2(:,i));
            ycorr_2(:,i) = (vd2(:,i)*radial_1(:,i));
        end
        Y_o3m=ycorr_2-ycorr_1;
        X_o3m=xcorr_2-xcorr_1;
        theta_o3m=atan2(Y_o3m,X_o3m);
        theta_o3m=theta_o3m*180/pi;
        
        
%Lag time Calculations
        t_range=0:40e-3:280e-3;
        lag_time_new = zeros(0,3);
        for i=1:numel(t_range)
            y_o = theta_o3m(i);
            x_o = t_range(i);
            % check if we find y0 <= y <= y1 in theta_ids
            k = find((theta_ids(1:end-1) <= y_o & theta_ids(2:end) >= y_o) | (theta_ids(1:end-1) >= y_o & theta_ids(2:end) <= y_o));
            if numel(k) == 1
                y_i = theta_ids(k:k+1);
                x_i = t_range(k:k+1);
                x_ii = interp1(y_i, x_i, y_o);
                lag_time_new = [lag_time_new; x_ii, x_ii - x_o, y_o];
            end
         end

        %lag_time_new = interp1(theta_o3m, t_range, theta_ids);
        delta_t=40e-3;
        %theta_o3m(4)-theta_o3m(5)
        
        figure;
        clf;
        plot(t_range,theta_ids,'go-',t_range,theta_o3m,'bo-');
        hold on
        l = line([lag_time_new(:,1), lag_time_new(:,1)-lag_time_new(:,2)]', [lag_time_new(:,3), lag_time_new(:,3)]');
        xlabel('Delta T (s)');
        ylabel('Theta (deg)');
        legend(l, cellfun( @(x) sprintf('%.1f ms ', x), num2cell(lag_time_new(:,2)*1000), 'UniformOutput', 0 ) );

end