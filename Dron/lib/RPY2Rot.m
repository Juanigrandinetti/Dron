%% matriz de rotación para convertir coordenadas solidarias al cuerpo del dron a coordenadas del sistema inercial
function bRi = RPY2Rot(angulos)
    phi = angulos(1);
    theta = angulos(2);
    psi = angulos(3);

    R_z = [cos(psi) sin(psi) 0; -sin(psi) cos(psi) 0; 0 0 1];
    R_y = [cos(theta) 0 -sin(theta); 0 1 0; sin(theta) 0 cos(theta)];
    R_x = [1 0 0; 0 cos(phi) sin(phi); 0 -sin(phi) cos(phi)];
    
    bRi = R_x*R_y*R_z;
end