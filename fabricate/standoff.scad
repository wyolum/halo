$fn=6;
inch = 25.4;
H = 15.5;

module standoff(h, d, D){
  translate([0, 0, h])cylinder(h=1.5, d=3, $fn=100);
  difference(){
    cylinder(h=h, d=D);
    translate([0, 0, -1])cylinder(h=h, d=d, $fn=6);
  }
}

standoff(H, 5.5, 9);
