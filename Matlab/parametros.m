clc
%% Parámetros de bases
Ts      = 0.0001;    % período de muestreo
k_st    = 5000;     % ganancia del controlador
%p       = 0.1;        % polo del error
%sat     = 200;      % saturación de la acción de control y del integrador del controlador

%% Parámetros del dron
%   Suponiendo un motor brushless de 10.000 RPM -> 1047 rad/s
%   Suponiendo una aceleración de 3G netos -> masa del dron x (4G - 1G)
%   Suponiendo un peso de 1kg aprox -> masa = 0.1 kg
%   Fuerza del dron 4kt omega^2 = 0.1 kg * 4 * 9.81m/s^2 = 3.924 N
%   => kt = 3.924 N / (4 omega^2) = 3.924 N / (4 * 1047^2) = 9e-7

Kt = 9e-7; % Constante de empuje de los motores
g = 9.81; % Gravedad
m = 0.1; % Masa
Ix = 0.11; Iy = Ix; Iz = 0.0075; % Inercia
l = 0.2; % Media longitud desde el centro hasta cualquier motor del dron
% kd (Revisar constante de que ??!!!!) = kt * l = 9e-7 * 0.2 = 1.8e-7
Kd = 1.8e-7;
kw = 1047; % Constante relacion tensión-motor, w(t) = k*U(t)

%% Parámetros de ruido
Pr = 0;

%% Z
Kpz = 0.1;
K1 = 0.0001; % Constante de proporcionalidad entre el error en z y T
%%

%% ganancias 1
alpha   = 1.1 * k_st;
lambda  = sqrt(k_st);

%% ganancias 2
% alpha   = 4 * k_st;
% lambda  = 0.5 * sqrt(k_st);

%% Parámetros Controlador Z

g_ez = 2;
kz_P = 9.6;
kz_I = 4.58*0.4;
kz_D = 2.52;

%% Parámetros Controlador Yaw

g_epsi = 0.5;
kpsi_P = 2;
kpsi_I = 0;
kpsi_D = 0;

%% Parámetros Controlador Pitch

g_ephi = 0.5;
kphi_P = 2;
kphi_I = 0;
kphi_D = 0;

%% Parámetros Controlador Roll

g_etheta = 0.5;
ktheta_P = 7.8*2.9;
ktheta_I = 2.95*0.01;
ktheta_D = 3.73*3;

%% Entradas a la planta
step_z     = 1; 
step_pitch = -pi/10;
step_roll  = pi/10;
step_yaw   = pi/4;

%% Valores de Referencia

z_ref     = 1;
psi_ref   = pi;
phi_ref   = pi/10;
theta_ref = pi/10;

