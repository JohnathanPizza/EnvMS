clc; clear; close all;

% Set up serial communication
arduino = serialport('COM5', 9600);
flush(arduino); % Clear any previous data

% Initialize arrays for storage
timeStamps = [];

% Air
CO2ppm = []; COppm = []; CH4ppm = []; O3ppb = [];
pm10ugm3 = []; pm25ugm3 = []; pm100ugm3 = [];

% Water
TDSppm = []; TurbidityPercent = []; pH = [];

% Environment
TempF = []; HumidityPercent = []; NoisedBA = [];
GPSlat = []; GPSlng = [];

% Set runtime duration
runTime = 900; % (Fix: was 'runtime' vs 'runTime')
startTime = tic;

while toc(startTime) < runTime
    if arduino.NumBytesAvailable > 0
        data = readline(arduino);
        values = str2double(split(data, ','));
        
        if numel(values) == 15 % Fix: was 'mumel'
            % Store values
            CO2ppm(end+1) = values(1);
            COppm(end+1) = values(2);
            CH4ppm(end+1) = values(3);
            O3ppb(end+1) = values(4);
            pm10ugm3(end+1) = values(5);
            pm25ugm3(end+1) = values(6);
            pm100ugm3(end+1) = values(7);
            TDSppm(end+1) = values(8);
            TurbidityPercent(end+1) = values(9);
            pH(end+1) = values(10);
            TempF(end+1) = values(11);
            HumidityPercent(end+1) = values(12);
            NoisedBA(end+1) = values(13);
            GPSlat(end+1) = values(14);
            GPSlng(end+1) = values(15);

            timeStamps(end+1) = toc(startTime);

            % --- Plot live air data ---
            figure(1); clf;

            % Subplot 1: CO2
            subplot(3,1,1);
            plot(timeStamps, CO2ppm, 'r', 'LineWidth', 2);
            y0 = CO2ppm;
            if ~isempty(y0) && min(y0) ~= 0 && max(y0) ~= 0
                ylim([min(y0)*0.8, max(y0)*1.2]);
            end
            xlabel('Time (s)'); ylabel('CO2 (ppm)');
            title('CO2');
            grid on;

            % Subplot 2: CO & CH4 (left), O3 (right)
            subplot(3,1,2);
            yyaxis left;
            plot(timeStamps, COppm, 'b-', 'LineWidth', 2); hold on;
            plot(timeStamps, CH4ppm, 'g-', 'LineWidth', 2);
            ylabel('CO & CH4 (ppm)');
            yyaxis right;
            plot(timeStamps, O3ppb, 'm--', 'LineWidth', 2);
            ylabel('O3 (ppb)');
            yyaxis left;
            y1 = [COppm, CH4ppm];
            if ~isempty(y1) && min(y1) ~= 0 && max(y1) ~= 0
                ylim([min(y1)*0.8, max(y1)*1.2]);
            end
            yyaxis right;
            y2 = O3ppb;
            if ~isempty(y2) && min(y2) ~= 0 && max(y2) ~= 0
                ylim([min(y2)*0.8, max(y2)*1.2]);
            end
            xlabel('Time (s)');
            title('CO, CH4, and O3');
            legend('CO', 'CH4', 'O3');
            grid on;

            % Subplot 3: Particulates
            subplot(3,1,3);
            plot(timeStamps, pm10ugm3, 'k-', 'LineWidth', 2); hold on;
            plot(timeStamps, pm25ugm3, 'c--', 'LineWidth', 2); hold on;
            plot(timeStamps, pm100ugm3, 'y:', 'LineWidth', 2);
            y3 = [pm10ugm3, pm25ugm3, pm100ugm3];
            if ~isempty(y3) && min(y3) ~= 0 && max(y3) ~= 0
                ylim([min(y3)*0.8, max(y3)*1.2]);
            end
            xlabel('Time (s)');
            ylabel('PM (\mug/m^3)');
            title('Particulate Matter');
            legend('PM1.0', 'PM2.5', 'PM10');
            grid on;

            % Overall title
            sgtitle('Air Pollutants Over Time');

            % --- Plot live water data ---
            figure(2); clf;

            % subplot 1: TDS (left), Turbidity (right)
            subplot(2,1,1);
            yyaxis left
            plot(timeStamps, TDSppm,'r-', 'LineWidth', 2); hold on;
            ylabel('TDS (ppm)');
            yyaxis right
            plot(timeStamps, TurbidityPercent,'b--', 'LineWidth', 2); hold on;
            ylabel('Turbidity (%)');
            yyaxis left
            y4 = TDSppm;
            if ~isempty(y4) && min(y4) ~= 0 && max(y4) ~= 0
                ylim([min(y4)*0.8, max(y4)*1.2]);
            end
            yyaxis right;
            y5 = TurbidityPercent;
            if ~isempty(y5) && min(y5) ~= 0 && max(y5) ~= 0
                ylim([min(y5)*0.8, max(y5)*1.2]);
            end
            xlabel('Time (s)');
            title('TDS and Turbidity');
            legend('TDS', 'Turbidity');
            grid on;

            % subplot 2: pH
            subplot(2,1,2);
            plot(timeStamps, pH, 'g', 'LineWidth', 2);
            y6 = pH;
            if ~isempty(y6) && min(y6) ~= 0 && max(y6) ~= 0
                ylim([min(y6)*0.8, max(y6)*1.2]);
            end
            xlabel('Time (s)'); ylabel('pH');
            title('pH');
            grid on;
            
            % Overall title
            sgtitle('Water Pollutants Over Time');

            % --- Plot live environmental data ---
            figure(3); clf;

            % subplot 1: Temperature (left), Humidity (right)
            subplot(2,1,1);
            yyaxis left
            plot(timeStamps, TempF,'k-', 'LineWidth', 2); hold on;
            ylabel('Temperature (F)');
            yyaxis right
            plot(timeStamps, HumidityPercent,'c--', 'LineWidth', 2); hold on;
            ylabel('Humidity (%)');
            yyaxis left
            y7 = TempF;
            if ~isempty(y7) && min(y7) ~= 0 && max(y7) ~= 0
                ylim([min(y7)*0.8, max(y7)*1.2]);
            end
            yyaxis right;
            y8 = HumidityPercent;
            if ~isempty(y8) && min(y8) ~= 0 && max(y8) ~= 0
                ylim([min(y8)*0.8, max(y8)*1.2]);
            end
            xlabel('Time (s)');
            title('Temperature and Humidity');
            legend('Temperature', 'Humidity');
            grid on;

            % subplot 2: Noise pollution
            subplot(2,1,2);
            plot(timeStamps, NoisedBA, 'y', 'LineWidth', 2);
            y9 = NoisedBA;
            if ~isempty(y9) && min(y9) ~= 0 && max(y9) ~= 0
                ylim([min(y9)*0.8, max(y9)*1.2]);
            end
            xlabel('Time (s)'); ylabel('Noise (dBA)');
            title('Noise Levels');
            grid on;
            
            % Overall title
            sgtitle('Environmental Data Over Time');
            
            % --- Plot live GPS Coordinates ---
            figure(4); clf;
            
            % Use dark blue track with red dot at current location
            geoplot(GPSlat, GPSlng, 'Color', [0.1 0.2 0.7], 'LineWidth', 2); hold on;
            geoscatter(GPSlat(end), GPSlng(end), 50, [1 0.2 0.2], 'filled');
            text(GPSlat(end), GPSlng(end), '  You Are Here', 'FontSize', 10);
            
            % Map and tight zooming
            geobasemap streets;
            if ~isempty(GPSlat) && ~isempty(GPSlng)
                padLat = 0.0003;
                padLng = 0.0003;
                geolimits([min(GPSlat)-padLat, max(GPSlat)+padLat], ...
                          [min(GPSlng)-padLng, max(GPSlng)+padLng]);
            end
            
            title('GPS Path on Map', 'FontWeight', 'bold');
           

            drawnow;
        end
    end
end

% Close serial
delete(arduino);
clear arduino;
