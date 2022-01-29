/* mandelbrot! */

x1 =  -2.0;  
y1 =  -2.0; 
x2 =   2.0; 
y2 =   2.0; 
px = 32;
py = 32;
for (y=0;y<py;y++)
{
    line="";
    for (x=0;x<px;x++)
    {
        xr=0;
        xi=0; 
        cr=x1+((x2-x1)*x/px);
        ci=y1+((y2-y1)*y/py);
        iterations=0;
        while ((iterations<32) && ((xr*xr+xi*xi)<4))
        {
            t=xr*xr - xi*xi + cr;
            xi=2*xr*xi+ci;          
            xr=t;        
            iterations++;
        }
        if (iterations & 1)
        {
            line += "*";
        }
        else
        {
            line += " ";
        }            
    }
   console.log(line);
   line = "";
}


