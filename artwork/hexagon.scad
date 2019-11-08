side_h = 21;
side_h_strut = 8;
side_w = 120;
side_d = 4;
side_h_offx = 15;
strut_w = 12;
strut_d = strut_w;
r = sqrt(3)/2*side_w;
rep_cnt = 4;
angle = 60/rep_cnt;
layers = 16;
display_h = 20;
display_w = 120;
display_d = 10;
display_offx = 5;
num_elems = 60/angle * layers/rep_cnt;
strut_h = side_h * num_elems;

strut_angle = 120;

for (b = [0 : strut_angle : 360-strut_angle]) {
    rotate([0, 0, b]) {
        for (a = [0 : 15 : 45]) {
            rotate([0, 0, a])
            translate([r-side_d-strut_d, -strut_w/2, 0])
            cube([strut_d, strut_w, strut_h]);
        }
    }
}

for (i = [0 : num_elems-1]) {
    translate([0, 0, side_h_offx + side_h * i])
    rotate([0, 0, angle*i]) {
        for (a = [0 : 60 : 300]) {
            rotate([0, 0, a])
            translate([r-side_d, -side_w/2, 0])
            cube([side_d, side_w, side_h_strut]);
        }
    }
}

color([0.8, 0.8, 0])
for (i = [0 : num_elems-1]) {
    translate([0, 0, display_offx + side_h * i])
    rotate([0, 0, angle*i]) {
        rotate([0, 0, a])
        translate([r, -display_w/2, 0])
        cube([display_d, display_w, display_h]);
    }
}