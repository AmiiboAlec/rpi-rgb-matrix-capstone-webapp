import {color, preview, get_color_from_color_input} from "./web_preview.js"

export class sequence extends preview {
    private submit_button: HTMLButtonElement
    private delay: number
    private frame_list: Array<Array<color>>
    private frame_index: number
    private selected_color: HTMLInputElement;
    private upload_button: HTMLInputElement;
    live_preview_button: HTMLInputElement
    live_preview_label: HTMLLabelElement

    public constructor (width: number, height: number, preview_area: HTMLDivElement) { //add image to this?
        let canvas = document.createElement("canvas");
        super(canvas, width, height);
        this.submit_button = document.createElement("button");
        this.submit_button.textContent = "Submit";
        this.frame_index = 0;
        this.frame_list = [[]];
        this.selected_color = document.createElement("input")
        this.selected_color.type = "color"
        this.selected_color.value = "#FF0000"
        let next_frame = document.createElement("button");
        let prev_frame = document.createElement("button");
        next_frame.textContent = "Next Frame";
        prev_frame.textContent = "Previous Frame";
        next_frame.onclick = () => {this.draw_next_frame()}
        prev_frame.onclick = () => {this.draw_previous_frame()}
        this.upload_button = document.createElement("input");
        this.upload_button.type = "file"

        canvas.onclick = (event) => {
            this.handle_mouse(event);
        }
        canvas.onmousemove = (event) => {
            if (event.buttons & 1) {
                canvas.onclick(event);
            }
        }
        this.submit_button.onclick = () => {this.submit()};
        
        let preview_selector_region = document.createElement('div');
        this.live_preview_button = document.createElement('input');
        this.live_preview_label = document.createElement('label');
        this.live_preview_label.innerHTML = "Enable live preview";
        this.live_preview_button.type = 'checkbox';
        this.live_preview_button.onclick = () => {
            if (this.live_preview_button.checked) {
                this.enable_preview();
            }
            else {
                this.disable_preview();
            }
        }
        preview_selector_region.appendChild(this.live_preview_label);
        preview_selector_region.appendChild(this.live_preview_button);

        preview_area.appendChild(this.selected_color);
        preview_area.appendChild(document.createElement("br"));
        preview_area.appendChild(canvas);
        preview_area.appendChild(document.createElement("br"));
        preview_area.appendChild(prev_frame);
        preview_area.appendChild(next_frame);
        preview_area.appendChild(document.createElement("br"));
        preview_area.appendChild(document.createElement("br"));
        preview_area.appendChild(this.upload_button);
        preview_area.appendChild(this.submit_button);
        preview_area.appendChild(preview_selector_region)

    }

    public set_delay(delay: number) {
        this.delay = delay;
    }

    public handle_mouse (event: MouseEvent) {
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
        
        let newColor = get_color_from_color_input(this.selected_color);

        this.set_pixel(raster_coordinates.x, raster_coordinates.y, newColor);
    }

    public toString() {
        let output_array: Array<string> = [];
        this.frame_list.forEach((frame) => {
            let temparray: Array<string> = [];
            for (let i = 0; i < frame.length; i++) {
                temparray.push(frame[i].toString());
            }
            output_array.push(temparray.join('|'));
        })
        return output_array.join('\n');
    }

    private draw_frame() {
        for (let y = 0; y < this.height; y++) {
            for (let x = 0; x < this.width; x++) {
                this.update_pixel(x, y, this.get_pixel(x,y));
            }
        }
    }

    public get_pixel(x: number, y: number) {
        return this.frame_list[this.frame_index][y * this.width + x];
    }

    private set_pixel(x: number, y: number, c: color) {
        this.frame_list[this.frame_index][y * this.width + x] = c;
        this.update_pixel(x, y, c);
    }

    public draw_next_frame() {
        if (++this.frame_index >= this.frame_list.length) this.frame_index = 0;
        this.draw_frame();
    }

    public draw_previous_frame() {
        if (--this.frame_index < 0) this.frame_index = this.frame_list.length - 1;
        this.draw_frame();
    }

    public submit() {
        const application_name = "new_gif_player";
        this.disable_preview();
        let starting_string = ["NEW", application_name, `Delay: ${this.delay}`].join('\n');
        let frame_data = this.toString().split('\n');
        this.sock.send(starting_string);
        frame_data.forEach((frame) => {this.sock.send(frame)});
        
    }

    public new_framelist(imgs: Array<HTMLImageElement>) {
        let resizer = document.createElement("canvas");
        let resizer_ctx = resizer.getContext("2d", {willReadFrequently: true});
        this.frame_list = [];
        imgs.forEach((image) => {
            resizer.width = image.width;
            resizer.height = image.height;
            resizer_ctx.drawImage(image, 0,0,this.width,this.height);
            let pixel_data = resizer_ctx.getImageData(0,0,this.width, this.height);
            let pixel_list = pixel_data.data;
            let output: Array<color>
            output = [];
            for (let i = 0; i < pixel_list.length; i += 4) {
                output.push(new color(pixel_list[i], pixel_list[i + 1], pixel_list[i + 2]))
            }
            this.frame_list.push(output);
        })
        this.frame_index = 0;
        this.draw_frame();
    }

    public new_image(img: HTMLImageElement) {
        this.new_framelist([img]);
    }
    
    public get_upload_button() { // I only need this because libgif doesn't have TS compatibility
        return this.upload_button;
    }
}