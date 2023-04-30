%%%% Creamos una clase para poder modificar los parámetros del dron.
classdef Dron < handle

    %% Miembros
    properties
        g %Aceleración de la gravedad
        t %Tiempo de simulación
        dt %Intervalo entre dos muestras de tiempo
        tf %Tiempo final de simulación
        m %Masa del dron
        l %Longitud del brazo
        I %Matriz de inercia
        x %Vector de estado [x, y, z, vx, vy, vz, phi, theta, psi, p, q, r]'
        r %[x, y, z]'
        dr %[dx, dy, dz]'
        euler %[phi, theta, psi]'
        w %[p, q, r]'
        dx %Variación del vector de estado
        u %Vector de control [T, M1, M2, M3]'
        T %Total thrust
        M %[M1, M2, M3]' = [tx, ty, tz]'
    end

    properties  
        phi_ref
        phi_err
        phi_err_prev
        phi_err_sum

        theta_ref
        theta_err
        theta_err_prev
        theta_err_sum

        psi_ref
        psi_err
        psi_err_prev
        psi_err_sum

        zdot_ref
        zdot_err
        zdot_err_prev
        zdot_err_sum

        z_ref
        z_err
        z_err_prev
        z_err_sum

        y_ref
        y_err
        y_err_prev
        y_err_sum

        x_ref
        x_err
        x_err_prev
        x_err_sum

        kP_phi
        kI_phi
        kD_phi

        kP_theta
        kI_theta
        kD_theta

        kP_psi
        kI_psi
        kD_psi

        kP_zdot
        kI_zdot
        kD_zdot

        kP_z
        kI_z
        kD_z 
    end

    %% Métodos
    methods
        %% Constructor (es un inicializador)
        %% Desde "main.m" se pueden crear objetos de tipo "Dron" con todos
        %% los atributos asociados a dicho objeto.
        function obj = Dron(param, estadoInicial, entradaInicial, gananciasControlador, tiempoSimulacion)
            obj.g = 9.81;
            obj.t = 0.0;
            obj.dt = 0.01;
            obj.tf = tiempoSimulacion;

            obj.m = param('Masa');
            obj.l = param('longBrazo');
            obj.I = [param('Ixx') 0 0; ...
                     0 param('Iyy') 0; ...
                     0 0 param('Izz')];
            
            obj.x = estadoInicial;
            obj.r = obj.x(1:3);
            obj.dr = obj.x(4:6);
            obj.euler = obj.x(7:9);
            obj.w = obj.x(10:12);
            
            %% Todavía no tenemos las ecuaciones de movimiento, por tanto, 
            %% inicializamos el vector derivada de las variables de estado
            %% a cero. 
            obj.dx = zeros(12,1);

            obj.u = entradaInicial;
            obj.T = obj.u(1);
            obj.M = obj.u(2:4);

            obj.phi_ref = 0.0;
            obj.phi_err = 0.0;
            obj.phi_err_prev = 0.0;
            obj.phi_err_sum = 0.0;

            obj.theta_ref = 0.0;
            obj.theta_err = 0.0;
            obj.theta_err_prev = 0.0;
            obj.theta_err_sum = 0.0;

            obj.psi_ref = 0.0; 
            obj.psi_err = 0.0;
            obj.psi_err_prev = 0.0;
            obj.psi_err_sum = 0.0;

            obj.zdot_ref = 0.0;
            obj.zdot_err = 0.0;
            obj.zdot_err_prev = 0.0;
            obj.zdot_err_sum = 0.0;

            obj.z_ref = 0.0;
            obj.z_err = 0.0;
            obj.z_err_prev = 0.0;
            obj.z_err_sum = 0.0;

            obj.y_ref = 0.0;
            obj.y_err = 0.0;
            obj.y_err_prev = 0.0;
            obj.y_err_sum = 0.0;

            obj.x_ref = 0.0;
            obj.x_err = 0.0;
            obj.x_err_prev = 0.0;
            obj.x_err_sum = 0.0;

            obj.kP_phi = gananciasControlador('P_phi');
            obj.kI_phi = gananciasControlador('I_phi');
            obj.kD_phi = gananciasControlador('D_phi');

            obj.kP_theta = gananciasControlador('P_theta');
            obj.kI_theta = gananciasControlador('I_theta');
            obj.kD_theta = gananciasControlador('D_theta');

            obj.kP_psi = gananciasControlador('P_psi');
            obj.kI_psi = gananciasControlador('I_psi');
            obj.kD_psi = gananciasControlador('D_psi');

            obj.kP_zdot = gananciasControlador('P_zdot');
            obj.kI_zdot = gananciasControlador('I_zdot');
            obj.kD_zdot = gananciasControlador('D_zdot');

            obj.kP_z = gananciasControlador('P_z');
            obj.kI_z = gananciasControlador('I_z');
            obj.kD_z = gananciasControlador('D_z');

        end
        

        function estado = getEstado(obj)
            estado = obj.x;
        end

        function obj = evalEOM(obj)
            % Cálculo de la velocidad, lineal y angular, y de la
            % aceleración, lineal y angular.
            bRi = RPY2Rot(obj.euler);
            R = bRi';

            obj.dx(1:3) = obj.dr; % Velocidad lineal
            obj.dx(4:6) = 1 / obj.m * ([0; 0; obj.m*obj.g] + R * obj.T * [0; 0; -1]); % Aceleración lineal

            phi = obj.euler(1); theta = obj.euler(2);

            %% Ahora obtenemos la derivada de los ángulos (phi punto, theta punto, psi punto)
            obj.dx(7:9) = [1 sin(phi)*tan(theta) cos(phi)*tan(theta); ...
                           0 cos(phi)            -sin(phi); ...
                           0 sin(phi)*sec(theta) cos(phi)*sec(theta)] * obj.w;

            %% Aceleración angular a partir de la ecuación de movimiento de Euler.
            obj.dx(10:12) = (obj.I) \ (obj.M - cross(obj.w, obj.I * obj.w));
        end

        function obj = updateEstado(obj)
            obj.t = obj.t + obj.dt;
            
            %% Calculamos el vector dx
            obj.evalEOM();

            %% Integración por método de Euler -->y_n+1 = y_n + dt*(y_n)'
            %% En 1 iteración, actualizamos el valor del vector x
            obj.x = obj.x + obj.dx.*obj.dt;
            
            %% Con el valor de x actualizado, actualizamos las demás variables
            obj.r = obj.x(1:3);
            obj.dr = obj.x(4:6);
            obj.euler = obj.x(7:9);
            obj.w = obj.x(10:12);
        end
        
        %% Controlador
        function obj = altitudCtrl(obj, yawReferencia)
            
%            obj.phi_ref = yawReferencia(1);
%            obj.theta_ref = yawReferencia(2);
%            obj.psi_ref = yawReferencia(3);
            obj.zdot_ref = yawReferencia;
            
            % Error = 'valor que quiero' - 'valor sensado'.
            obj.phi_err = obj.phi_ref - obj.euler(1);
            obj.theta_err = obj.theta_ref - obj.euler(2);
            obj.psi_err = obj.psi_ref - obj.euler(3);
            obj.zdot_err = obj.zdot_ref - obj.dr(3);

            obj.u(2) = (obj.kP_phi * obj.phi_err + ...
                        obj.kI_phi * obj.phi_err_sum + ...
                        obj.kD_phi * (obj.phi_err - obj.phi_err_prev)/obj.dt);

            obj.phi_err_prev = obj.phi_err;
            obj.phi_err_sum = obj.phi_err_sum + obj.phi_err;

            obj.u(3) = (obj.kP_theta * obj.theta_err + ...
            obj.kI_theta * obj.theta_err_sum + ...
            obj.kD_theta * (obj.theta_err - obj.theta_err_prev)/obj.dt);

            obj.theta_err_prev = obj.theta_err;
            obj.theta_err_sum = obj.theta_err_sum + obj.theta_err;
            
            obj.u(4) = (obj.kP_psi * obj.psi_err + ...
                        obj.kI_psi * obj.psi_err_sum + ...
                        obj.kD_psi * (obj.psi_err - obj.psi_err_prev)/obj.dt);

            obj.psi_err_prev = obj.psi_err;
            obj.psi_err_sum = obj.psi_err_sum + obj.psi_err;
            
            obj.u(1) = obj.m * obj.g - ((obj.kP_zdot * obj.zdot_err + ...
                       obj.kI_zdot * obj.zdot_err_sum + ...
                       obj.kD_zdot * (obj.zdot_err - obj.zdot_err_prev)/obj.dt));

            obj.zdot_err_prev = obj.zdot_err;
            obj.zdot_err_sum = obj.zdot_err_sum + obj.zdot_err;
            
            obj.T = obj.u(1);
            obj.M = obj.u(2:4);
        end

        function obj = PosCtrl(obj, posReferencia)
            obj.z_ref = posReferencia(3);
            obj.y_ref = posReferencia(2);
            obj.x_ref = posReferencia(1);
            
            obj.z_err = obj.z_ref - obj.r(3);
            obj.y_err = obj.y_ref - obj.r(2);
            obj.x_err = obj.x_ref - obj.r(1);

            obj.psi_ref = atan(obj.y_ref / obj.x_ref);

            obj.theta_ref = obj.kP_theta * obj.x_err + ...
                            obj.kI_theta * obj.x_err_sum + ...
                            obj.kD_theta * (obj.x_err - obj.x_err_prev)/obj.dt;
            
            obj.phi_ref = obj.kP_phi * obj.y_err + ...
                            obj.kI_phi * obj.y_err_sum + ...
                            obj.kD_phi * (obj.y_err - obj.y_err_prev)/obj.dt;

            obj.u(1) = obj.m * obj.g - ((obj.kP_z * obj.z_err + ...
                       obj.kI_z * obj.z_err_sum + ...
                       obj.kD_z * (obj.z_err - obj.z_err_prev)/obj.dt));

            obj.z_err_prev = obj.z_err;
            obj.z_err_sum = obj.z_err_sum + obj.z_err;

            obj.T = obj.u(1);
        end
    end
end
