import { color, preview, get_color_from_color_input } from "./web_preview.js";
import { fontpack } from "./vars.js";
const text_height = 8;
const text_width = 6;
class text extends preview {
    constructor(width, height, text_area) {
        //create elements and initalize variables
        let canvas = document.createElement("canvas");
        super(canvas, width, height);
        this.submit_button = document.createElement("button");
        this.submit_button.textContent = "Submit";
        this.preview_button = document.createElement("input");
        this.preview_button.type = "checkbox";
        this.preview_button.id = "preview button";
        let preview_label = document.createElement("label");
        preview_label.innerHTML = " Enable Live Preview";
        preview_label.appendChild(this.preview_button);
        this.rows = height / 8;
        this.text = [];
        this.text_color = [];
        this.text_forms = [];
        this.current_scroll_offset = [];
        this.scroll_value_input = [];
        this.selected_color = document.createElement("input");
        this.selected_color.type = "color";
        this.selected_color.value = "#FF0000";
        canvas.onclick = (event) => { this.handle_mouse(event); };
        this.submit_button.onclick = this.submit;
        this.preview_button.onclick = () => {
            if (this.preview_mode)
                this.disable_preview();
            else
                this.enable_preview();
        };
        //append elements to its div
        text_area.appendChild(this.selected_color);
        text_area.appendChild(document.createElement("br"));
        text_area.appendChild(document.createElement("br"));
        text_area.appendChild(canvas);
        text_area.appendChild(document.createElement("br"));
        for (let i = 0; i < this.rows; i++) {
            let textbox_label = document.createElement("label");
            textbox_label.innerHTML = `Row ${i + 1} text:`;
            let scrollbox_label = document.createElement("label");
            scrollbox_label.innerHTML = `     Row ${i + 1} scroll speed (in pixels per second):`;
            this.text.push("");
            this.text_color.push([]);
            this.current_scroll_offset.push(0);
            let text_row = document.createElement("input");
            let text_scroll_input = document.createElement("input");
            textbox_label.appendChild(text_row);
            scrollbox_label.appendChild(text_scroll_input);
            text_row.type = "text";
            text_row.onchange = () => {
                this.text[i] = text_row.value;
                //console.log(text_row.value);
                this.text_color[i] = new Array(this.text[i].length);
                for (let j = 0; j < this.text_color[i].length; j++) {
                    this.text_color[i][j] = get_color_from_color_input(this.selected_color);
                    //console.log("did this even update");
                }
                this.draw_row(i); //this shouldn't work
            };
            text_scroll_input.type = "number";
            this.text_forms.push(text_row);
            this.scroll_value_input.push(text_scroll_input);
            text_area.appendChild(textbox_label);
            text_area.appendChild(scrollbox_label);
            text_area.appendChild(document.createElement("br"));
            text_area.appendChild(document.createElement("br"));
        }
        text_area.appendChild(this.submit_button);
        text_area.appendChild(document.createElement("br"));
        text_area.appendChild(document.createElement("br"));
        text_area.appendChild(preview_label);
    }
    get_selected_char(x, y) {
        let row = Math.floor(y / text_height);
        let x_with_scroll = x + this.current_scroll_offset[row];
        let column = Math.floor(x_with_scroll / text_width);
        return { row: row, column: column };
    }
    draw_char(row, char, starting_position) {
        //console.log(`Drawing Row ${row} Char ${char}`)
        const row_offset = row * text_height;
        //const column_offset = char * text_width - this.current_scroll_offset[row];
        const column_offset = starting_position;
        function draw_column(column, top_row, font_column, t) {
            //console.log(font_column);
            for (let i = 0; i < text_height; i++) { //something is wrong here
                if (((font_column >> (text_height - i - 1)) & 1) == 1) {
                    //console.log("pixel on");
                    t.update_pixel(column, top_row + i, t.text_color[row][char % t.text[row].length]);
                }
                else {
                    t.update_pixel(column, top_row + i, new color(0, 0, 0));
                }
            }
        }
        let char_to_draw = this.text[row].charCodeAt(char % this.text[row].length);
        for (let i = column_offset; i < column_offset + text_width; i++) {
            if ((i < 0) || (i > this.width))
                continue;
            draw_column(i, row_offset, fontpack[char_to_draw][i - column_offset], this);
        }
    }
    handle_mouse(event) {
        let mouse_to_pix_xy = function (event, c) {
            let rect = c.getBoundingClientRect();
            let mouseX = event.clientX - rect.left;
            let mouseY = event.clientY - rect.top;
            let radius = c.width / 128;
            let output = { x: 0, y: 0 };
            output.x = Math.floor(mouseX / (2 * radius));
            output.y = Math.floor(mouseY / (2 * radius));
            return output;
        };
        let raster_coordinates = mouse_to_pix_xy(event, this.canvas);
        //console.log(raster_coordinates)
        let selected_char = this.get_selected_char(raster_coordinates.x, raster_coordinates.y);
        if (raster_coordinates.x == 0) { //scroll row to the left
            //console.log("first if");
            this.current_scroll_offset[selected_char.row]--;
            this.draw_row(selected_char.row);
        }
        else if (raster_coordinates.x == this.width - 1) { //scroll row to the right
            //console.log("second if")
            this.current_scroll_offset[selected_char.row]++;
            this.draw_row(selected_char.row);
        }
        else {
            //console.log("general case");
            let col = get_color_from_color_input(this.selected_color);
            //console.log(col)
            this.text_color[selected_char.row][selected_char.column % this.text[selected_char.row].length] = col;
            this.draw_row(selected_char.row);
        }
    }
    toString() {
        let output_array;
        for (let i = 0; i < this.rows; i++) {
            output_array.push(this.text[i]);
            let color_string_array;
            for (let h = 0; h < this.text_color[i].length; h++) {
                color_string_array.push(this.text_color[i][h].toString());
            }
            output_array.push(color_string_array.join());
        }
        return output_array.join('\n');
    }
    submit() {
        const application_name = "TBA"; //Text mode not yet implemented, name of the executable unclear currently
        let starting_string = ["NEW", application_name].join('\n');
        let data_string = this.toString();
        this.sock.send(starting_string);
        this.sock.send(data_string);
    }
    draw_row(row) {
        //find first visible char
        let scroll = this.current_scroll_offset[row];
        let first_visible_char = (Math.floor(scroll / text_width)) % this.text[row].length;
        let offset = ((this.current_scroll_offset[row] * -1) - text_width) % text_width;
        //console.log(first_visible_char);
        if (first_visible_char < 0)
            first_visible_char += this.text[row].length;
        if (offset > 0)
            offset -= text_width;
        for (let i = first_visible_char; i < 1 + (first_visible_char + (this.width / text_width)); i++) {
            //console.log(`Drawing ${this.text[row][i % this.text[row].length]} at location ${offset + (i * text_width)}. Offset is ${offset}`)
            this.draw_char(row, i, offset + ((i - first_visible_char) * text_width));
        }
    }
}
let canvas_state;
window.onload = () => {
    let preview_div = document.getElementById("PreviewDiv");
    canvas_state = new text(64, 32, preview_div);
};
//# sourceMappingURL=text_mode.js.map