import {color, preview, get_color_from_color_input} from "./web_preview.js"
import {testPixel, fontpack} from "./vars.js"

const text_height = 8
const text_width = 6

class text extends preview {
    private text: Array<string>
    private text_color: Array<Array<color>>
    private background_color: Array<Array<color>>
    private submit_button: HTMLButtonElement
    private rows: number
    private text_forms: Array<HTMLInputElement>
    private scroll_value_input: Array<HTMLInputElement>
    private current_scroll_offset: Array<number>
    private selected_color: HTMLInputElement
    private preview_button: HTMLInputElement

    constructor(width: number, height: number, text_area: HTMLDivElement) {
        //create elements and initalize variables
        let canvas = document.createElement("canvas");
        super(canvas, width, height);
        this.submit_button = document.createElement("button");
        this.submit_button.textContent = "Submit";
        this.preview_button = document.createElement("input");
        this.preview_button.type = "checkbox";
        this.preview_button.id = "preview button";
        let preview_label = document.createElement("label");
        preview_label.innerHTML = " Enable Live Preview"
        preview_label.appendChild(this.preview_button);
        this.rows = height / 8;
        this.text = [];
        this.text_color = [];
        this.background_color = [];
        this.text_forms = [];
        this.current_scroll_offset = [];
        this.scroll_value_input = [];
        this.selected_color = document.createElement("input");
        this.selected_color.type = "color";
        this.selected_color.value = "#FF0000"
        canvas.onclick = (event) => {this.handle_mouse(event)};
        this.submit_button.onclick = () => {this.submit()};
        this.preview_button.onclick = () => {
            if (this.preview_mode) this.disable_preview()
            else this.enable_preview()
        }

        //append elements to its div
        text_area.appendChild(this.selected_color);
        text_area.appendChild(document.createElement("br"))
        text_area.appendChild(document.createElement("br"))
        text_area.appendChild(canvas);
        text_area.appendChild(document.createElement("br"));
        for (let i = 0; i < this.rows; i++) {
            let textbox_label = document.createElement("label");
            textbox_label.innerHTML = `Row ${i + 1} text:`
            let scrollbox_label = document.createElement("label");
            scrollbox_label.innerHTML = `     Row ${i+1} scroll speed (in pixels per second):`
            this.text.push("");
            this.text_color.push([]);
            this.background_color.push([]);
            this.current_scroll_offset.push(0);
            let text_row = document.createElement("input")
            let text_scroll_input = document.createElement("input")
            textbox_label.appendChild(text_row);
            scrollbox_label.appendChild(text_scroll_input);
            text_row.type = "text";
            text_row.onchange = () => {
                this.text[i] = text_row.value; 
                //console.log(text_row.value);
                this.text_color[i] = new Array<color>(this.text[i].length);
                this.background_color[i] = new Array<color>(this.text[i].length);
                for (let j = 0; j < this.text_color[i].length; j++) {
                    this.text_color[i][j] = get_color_from_color_input(this.selected_color)
                    this.background_color[i][j] = new color(0,0,0);
                    //console.log("did this even update");
                }
                this.draw_row(i);    //this shouldn't work
            }
            text_scroll_input.type = "number";
            text_scroll_input.value = '0';
            this.text_forms.push(text_row);
            this.scroll_value_input.push(text_scroll_input);
            text_area.appendChild(textbox_label);
            text_area.appendChild(scrollbox_label);
            text_area.appendChild(document.createElement("br"));
            text_area.appendChild(document.createElement("br"))
        }
        text_area.appendChild(this.submit_button);
        text_area.appendChild(document.createElement("br"))
        text_area.appendChild(document.createElement("br"))
        text_area.appendChild(preview_label);
    }

    private get_selected_char(x: number, y: number) {
        let row = Math.floor(y / text_height);
        let x_with_scroll = x + this.current_scroll_offset[row];
        let column = Math.floor(x_with_scroll / text_width);
        return {row: row, column: column}
    }

    private draw_char(row: number, char: number, starting_position: number) {
        //console.log(`Drawing Row ${row} Char ${char}`)
        const row_offset = row * text_height;
        //const column_offset = char * text_width - this.current_scroll_offset[row];
        const column_offset = starting_position
        function draw_column(column: number, top_row: number, font_column: number, t: text) {
            //console.log(font_column);
            for (let i = 0; i < text_height; i++) { //something is wrong here
                if (((font_column >> (text_height - i - 1)) & 1) == 1){
                    //console.log("pixel on");
                    t.update_pixel(column, top_row + i, t.text_color[row][char % t.text[row].length]);
                }
                else t.update_pixel(column, top_row + i, t.text_color[row][char % t.text[row].length]);
            }
        }
        let char_to_draw = this.text[row].charCodeAt(char % this.text[row].length);
        for (let i = column_offset; i < column_offset + text_width; i++) {
            if ((i < 0) || (i > this.width)) continue;
            
            draw_column(i, row_offset, fontpack[char_to_draw][i - column_offset], this);
        }
    }

    private handle_mouse (event: MouseEvent) {
        let mouse_to_pix_xy = (event: MouseEvent, c: HTMLCanvasElement) => {
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
        //console.log(raster_coordinates)
        let selected_char = this.get_selected_char(raster_coordinates.x,raster_coordinates.y);
        if (raster_coordinates.x == 0) { //scroll row to the left
            //console.log("first if");
            this.current_scroll_offset[selected_char.row]--;
            this.draw_row(selected_char.row);
        }
        else if (raster_coordinates.x == this.width - 1) {   //scroll row to the right
            //console.log("second if")
            this.current_scroll_offset[selected_char.row]++;
            this.draw_row(selected_char.row);
        }
        else {
            //console.log("general case");
            let col = get_color_from_color_input(this.selected_color);
            //console.log(col)
            let index_into_screen_space = raster_coordinates.x;
            let row = Math.floor(raster_coordinates.y / text_height);
            let index_into_pixel_string = index_into_screen_space + this.current_scroll_offset[row];
            index_into_pixel_string %= this.text[row].length * text_width;
            if (index_into_pixel_string < 0) index_into_pixel_string += this.text[row].length * text_width;
            let index_into_char_string = Math.floor(index_into_pixel_string / text_width);
            let index_into_font = index_into_pixel_string % text_width;
            let char_to_draw = this.text[row][index_into_char_string];
            let is_char = fontpack[char_to_draw.charCodeAt(0)][index_into_font] & (1 << (text_height - (raster_coordinates.y % text_height) - 1));
            if (is_char) {
                this.text_color[selected_char.row][selected_char.column % this.text[selected_char.row].length] = col;
            }
            else {
                this.background_color[selected_char.row][selected_char.column % this.text[selected_char.row].length] = col;
            }
            this.draw_row(selected_char.row);
        }
    }

    public toString() { //Does not yet include scroll value
        let output_array: Array<string> = []
        for (let i = 0; i < this.rows; i++) {
            let temp_array: Array<string> = []
            temp_array.push(this.text[i]);
            let color_string_array1: Array<string> = []
            let color_string_array2: Array<string> = []
            for (let h = 0; h < this.text_color[i].length; h++) {
                color_string_array1.push(this.text_color[i][h].toString());
                color_string_array2.push(this.background_color[i][h].toString());
            }
            temp_array.push(color_string_array1.join('|'));
            temp_array.push(color_string_array2.join('|'));
            temp_array.push(this.scroll_value_input[i].value)
            output_array.push(temp_array.join('\t'));
        }
        return output_array.join('\n');
    }

    public submit() {
        const application_name = "text_mode"; //Text mode not yet implemented, name of the executable unclear currently
        let starting_string = ["NEW", application_name].join('\n');
        let data_string = this.toString();
        this.sock.send(starting_string + '\n' + data_string);
        //console.log(starting_string + '\n' + data_string);
    }

    private draw_row(row: number) {
        //find first visible char
        let scroll = this.current_scroll_offset[row];
        let draw_column = (start: number, bitmap: number, foreground_color: color, bg_color: color) => {
            let x = start % this.width;
            let y = Math.floor(start / this.width);
            for (let i = 0; i < text_height; i++) {
                let is_pixel_on: boolean;
                is_pixel_on = (bitmap & (0x80)) != 0;
                bitmap = bitmap << 1;
                if (is_pixel_on) {
                    this.update_pixel(x, y + i, foreground_color);
                }
                else this.update_pixel(x, y + i, bg_color);
            }
        }
        for (let i = 0; i < this.width; i++) {
            let index_into_pixel_string = this.current_scroll_offset[row] + i;
            index_into_pixel_string %= this.text[row].length * text_width;
            if (index_into_pixel_string < 0) index_into_pixel_string += this.text[row].length * text_width;
            let index_into_char_string = Math.floor(index_into_pixel_string / text_width);
            let index_into_font = index_into_pixel_string % text_width;
            let char_to_draw = this.text[row][index_into_char_string];
            draw_column(i + (row * this.width * text_height), fontpack[char_to_draw.charCodeAt(0)][index_into_font], this.text_color[row][index_into_char_string], this.background_color[row][index_into_char_string]);
        }
        
    }   
}

let canvas_state: text;

window.onload = () => {
    let preview_div = <HTMLDivElement>document.getElementById("PreviewDiv");
    canvas_state = new text(64, 32, preview_div); 
}