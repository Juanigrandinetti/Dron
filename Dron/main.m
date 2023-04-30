close all;
clc; clear;

addpath('./lib');

%%%% Conversión entre radianes y ángulos.

R2D = 180/pi;
D2R = pi/180;

%%% Inicializamos parámetros del dron.

dron1_param = containers.Map({'Masa', 'longBrazo', 'Ixx', 'Iyy', 'Izz'}, ...
    {1.25, 0.265, 0.0232, 0.0232, 0.0468});

dron1_estadoInicial = [0, 0, -8, ... %x, y, z
    0, 0, 0, ...                    %vx, vy, vz
    0, 0, 0, ...                    %phi, theta, psi [Ángulos de Euler]
    0, 0, 0]';                      %p, q, r [velocidad angular en el sistema de referencia solidario al dron]

dron1_entradaInicial = [0, 0, 0, 0]'; %u1, u2, u3, u4 (T, M1, M2, M3) -->Varaibles de control
                                      %T: total thrust
                                      %M1 = tx, M2 = ty, y M3 = tz 
                                      % son los torques en cada eje

dron1_cuerpo = [0.0265,      0,     0, 1; ...
                     0, -0.265,     0, 1; ...
                -0.265,      0,     0, 1; ...
                     0,  0.265,     0, 1; ...
                     0,      0,     0, 1; ...
                     0,      0, -0.15, 1]';

dron1_gananciasControlador = containers.Map(...
    {'P_phi', 'I_phi', 'D_phi', ...
    'P_theta', 'I_theta', 'D_theta', ...
    'P_psi', 'I_psi', 'D_psi', ...
    'P_zdot', 'I_zdot', 'D_zdot'}, ...
    {0.0, 0.0, 0.0, ...
     0.0, 0.0, 0.0, ...
     0.0, 0.0, 0.0, ...
     10.0, 0.0, 0.0});

tiemposimulacion = 10;
dron1 = Dron(dron1_param, dron1_estadoInicial, dron1_entradaInicial, dron1_gananciasControlador, tiemposimulacion);

%% Definimos el espacio en 3D
fig1 = figure('pos', [0 200 800 800]);
h = gca;
view(3);
fig1.CurrentAxes.ZDir = 'Reverse';
fig1.CurrentAxes.YDir = 'Reverse';

axis equal;
grid on

xlim([-5 5]); ylim([-5 5]); zlim([-10 0]);
xlabel('X[m]'); ylabel('Y[m]'); zlabel('Z[m]');

hold(gca, 'on');

% Obtenemos las variables de estado
dron1_estado = dron1.getEstado();
% Matriz homogénea
wHb = [RPY2Rot(dron1_estado(7:9))' dron1_estado(1:3); ...
       0 0 0 1];
dron1_inercial = wHb * dron1_cuerpo;
dron1_altura = dron1_inercial(1:3, :);

% Creamos el cuerpo del dron
fig1_BRAZO13= plot3(gca, dron1_altura(1, [1 3]), ...
                         dron1_altura(2, [1 3]), ...
                         dron1_altura(3, [1 3]), ...
                         '-yo', 'MarkerSize', 5); % Conectamos los motores 1 y 3
fig1_BRAZO24 = plot3(gca, dron1_altura(1, [2 4]), ...
                          dron1_altura(2, [2 4]), ...
                          dron1_altura(3, [2 4]), ...
                         '-bo', 'MarkerSize', 5); % Conectamos los motores 2 y 4
fig1_payload = plot3(gca, dron1_altura(1, [5 6]), ...
                          dron1_altura(2, [5 6]), ...
                          dron1_altura(3, [5 6]), ...
                         '-k', 'LineWidth', 3);
fig1_sombra = plot3(gca,0,0,0,'xk','LineWidth',3);

hold(gca, 'off');

%% Definimos datos de la figura
fig2 = figure('pos',[800 500 800 500]);
subplot(3,3,1);
title('phi[deg]');
grid on;
hold on;
subplot(3,3,2);
title('theta[deg]');
grid on;
hold on;
subplot(3,3,3);
title('psi[deg]');
grid on;
hold on;
subplot(3,3,4);
title('x[m]');
grid on;
hold on;
subplot(3,3,5);
title('y[m]');
grid on;
hold on;
subplot(3,3,6);
title('zdot[m/s]');
grid on;
hold on;
subplot(3,3,7);
title('z[m]');
grid on;
hold on;
%% 
commandSig(1) = 0.0 * D2R;
commandSig(2) = 0.0 * D2R;
commandSig(3) = 0.0 * D2R;
commandSig(4) = 1.0;

for i = 1:tiemposimulacion/0.01
    dron1.altitudCtrl(commandSig);
    % El dron actualiza su estado en función de los torques y la
    % propulsión (thrsut)
    dron1.updateEstado();

    dron1_estado = dron1.getEstado();

    %% Gráfico 3D
    figure(1);
    wHb = [RPY2Rot(dron1_estado(7:9))' dron1_estado(1:3); 0 0 0 1];
    dron1_inercial = wHb * dron1_cuerpo;
    dron1_altura = dron1_inercial(1:3, :);

    set(fig1_BRAZO13, ...
        'XData', dron1_altura(1, [1 3]), ...
        'YData', dron1_altura(2, [1 3]), ...
        'ZData', dron1_altura(3, [1 3]));

    set(fig1_BRAZO24, ...
        'XData', dron1_altura(1, [2 4]), ...
        'YData', dron1_altura(2, [2 4]), ...
        'ZData', dron1_altura(3, [2 4]));

    set(fig1_payload, ...
        'XData', dron1_altura(1, [5 6]), ...
        'YData', dron1_altura(2, [5 6]), ...
        'ZData', dron1_altura(3, [5 6]));

    set(fig1_sombra, ...
        'XData', dron1_estado(1), ...
        'YData', dron1_estado(2), ...
        'ZData', 0);
    
    figure(2)
    subplot(3,3,1);
        plot(i/100, dron1_estado(7)*R2D, '.');
    subplot(3,3,2);
        plot(i/100, dron1_estado(8)*R2D, '.');
    subplot(3,3,3);
        plot(i/100, dron1_estado(9)*R2D, '.');
    subplot(3,3,4);
        plot(i/100, dron1_estado(1), '.');
    subplot(3,3,5);
        plot(i/100, dron1_estado(2), '.');
    subplot(3,3,6);
        plot(i/100, dron1_estado(6), '.');
    subplot(3,3,7);
        plot(i/100, dron1_estado(3), '.');

    drawnow;
    if(dron1_estado(3) >= 0)
        msgbox('Crashed!', 'Error', 'error');
        break;
    end
end








