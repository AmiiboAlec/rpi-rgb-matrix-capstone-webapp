
export class color {
    r: number;
    g: number;
    b: number;

    constructor (red: number, green: number, blue: number) {
        if (red > 255) red = 255;
        if (green > 255) green = 255;
        if (blue > 255) blue = 255;
        if (red < 0) red = 0;
        if (blue < 0) blue = 0;
        if (green < 0) green = 0;
        this.r = red;
        this.g = green;
        this.b = blue;
    }
    
    public toString() {
        let convert_hex = (val: number) => {
            return Math.round(val).toString(16).padStart(2,'0');
        }
        return convert_hex(this.r) + convert_hex(this.g) + convert_hex(this.b);
    }
}

export function get_color_from_color_input(input_element: HTMLInputElement) {
    let text = input_element.value;
    let red = parseInt(text.substring(1, 3), 16);
    let green = parseInt(text.substring(3,5),16);
    let blue = parseInt(text.substring(5,7),16);
    return new color(red, green, blue);
}

export class preview {
    private dot_copier: HTMLCanvasElement
    protected canvas: HTMLCanvasElement
    private ctx: CanvasRenderingContext2D | null
    private framebuffer: Array<color>
    protected width: number;
    protected height: number;
    private onclick_function: (x: number, y: number) => void
    protected preview_mode: boolean
    protected sock: WebSocket | null

    public constructor (canvas: HTMLCanvasElement, width: number, height: number) {
        const hostname = "ledtest.local"
        const port_number = 9002
        this.canvas = canvas;
        this.ctx = canvas.getContext("2d");
        //this.resize_canvas();
        if (this.ctx == null) throw "Null Canvas Error";
        this.framebuffer = [];
        this.width = width;
        this.height = height;
        for (let i = 0; i < width * height; i++) {
            this.framebuffer.push(new color(0, 0, 0));
        }
        this.preview_mode = false;
        this.sock = new WebSocket(`ws://${hostname}:${port_number}`)
        
        window.addEventListener("resize", () => {this.resize_canvas()});

        this.resize_canvas();
    }

    public update_pixel(x: number, y: number, color: color) {
        if (color == undefined) {
            console.log("Undefined color");
            return
        }
        this.framebuffer[y * this.width + x] = color;
        this.redraw_pixel(x,y);
    }

    public set_onclick_event_function(callback: (x: number, y:number) => void) {
        this.onclick_function = callback;
    }

    private redraw_pixel(x: number, y: number) { //uses raster coordinates; (0,0) is top left.
        let c = this.framebuffer[y * this.width + x];
        /*const diameter = this.canvas.width / this.width;
        const leftx = diameter * x;
        const upy = diameter * y;
        this.ctx.drawImage(this.dot_copier, leftx, upy);
        this.ctx.globalCompositeOperation = "source-in";
        this.ctx.fillStyle = `rgb(${c.r}, ${c.g}, ${c.b})`
        this.ctx.fillRect(0,0,this.canvas.width, this.canvas.height);

        return;*/
        const radius = this.canvas.width / 128;
        const centerx = radius * (x * 2 + 1);
        const centery = radius * (y * 2 + 1);
        if (this.preview_mode === true) {
            let msg = `Pixel ${x},${y}: ${c.toString()}`
            console.log(msg);
            this.sock.send(msg)
        }
        
        this.ctx?.beginPath();
        this.ctx?.arc(centerx, centery, radius, 0, 2 * Math.PI, false);

        this.ctx!.fillStyle = `rgb(${c.r}, ${c.g}, ${c.b})`
        this.ctx?.fill();
        
    }
    
    /*private handle_mouse(event: MouseEvent) {
        let mouse_to_pix_xy = function(event: MouseEvent, c: HTMLCanvasElement) {
            let rect = c.getBoundingClientRect();
            let mouseX = event.clientX - rect.left;
            let mouseY = event.clientY - rect.top;
            let radius = c.width / 128;
            let output = {x:0,y:0};
            output.x = Math.floor(mouseX / (2 * radius));
            output.y = Math.floor(mouseY / (2 * radius));
            return output;
        }
        
        let raster_coordinates = mouse_to_pix_xy(event, this.canvas);

        this.onclick_function(raster_coordinates.x, raster_coordinates.y);
    }*/

    private draw_background() {
        this.ctx?.beginPath();
        this.ctx!.fillStyle = 'darkgrey';
        this.ctx?.rect(0,0,this.canvas.width, this.canvas.height);
        this.ctx?.fill();
    }

    private redraw () {
        this.draw_background();
        for (let y = 0; y < this.height; y++) {
            for (let x = 0; x < this.width; x++) {
                this.redraw_pixel(x, y);
            }
        }
    }

    public enable_preview() {
        //this.sock = sock;
        this.sock.send("NEW\nnew_live_preview");
        this.preview_mode = true;
    }

    public disable_preview() {
        this.preview_mode = false;
    }

    public resize_canvas() { //include this in window.onresize
        let size_param = (window.innerHeight < window.innerWidth) ? window.innerHeight : window.innerWidth;
        this.canvas.width = size_param * 0.8;
        this.canvas.height = size_param * 0.4;
        /*this.dot_copier = document.createElement('canvas');
        document.body.append(this.dot_copier);
        const diameter = this.canvas.width / 128;
        this.dot_copier.width = diameter;
        this.dot_copier.height = diameter;
        let ctx = this.dot_copier.getContext("2d");
        ctx!.fillStyle = "rgba(255,255,255,0)";
        ctx?.fillRect(0,0,diameter, diameter);
        ctx?.beginPath();
        ctx?.arc(diameter / 2, diameter / 2, diameter / 2, 0, 2 * Math.PI, false);
        ctx!.fillStyle = "rgb(0,0,0)";
        ctx?.fill();*/

        
        this.redraw();
    }
}