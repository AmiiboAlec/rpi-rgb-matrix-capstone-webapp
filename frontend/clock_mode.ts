
//let sock: WebSocket;
/*let inside_color_picker: HTMLInputElement;
let outside_color_picker: HTMLInputElement;
let dot_quantity_picker: HTMLInputElement;
let time_scale_picker: HTMLInputElement;
let decay_rate_picker: HTMLInputElement;
*/

window.onload = () => {
    let dumpingground = <HTMLDivElement> document.getElementById("Form div");

    let inside_color_picker = document.createElement("input");
    inside_color_picker.type="color";
    inside_color_picker.value = "#39FF14";

    let outside_color_picker = document.createElement("input");
    outside_color_picker.type="color";
    outside_color_picker.value="#A3FFF9";

    let dot_quantity_picker = document.createElement("input");
    dot_quantity_picker.type="number";
    dot_quantity_picker.step="1";
    dot_quantity_picker.min="1";
    dot_quantity_picker.value="4";

    let time_scale_picker=document.createElement("input");
    time_scale_picker.type="number";
    time_scale_picker.value="4";
    time_scale_picker.min="0";

    let decay_rate_picker=document.createElement("input");
    decay_rate_picker.type="number";
    decay_rate_picker.max="1";
    decay_rate_picker.value="0.25";

    let var_list = document.createElement("ul");
    var_list.style.listStyle="none";

    let inside_color_area = document.createElement("li");
    let inside_color_label = document.createElement("label");
    let inside_color_description = document.createElement('p');
    inside_color_label.innerHTML = "Hands Color: ";
    inside_color_description.innerHTML = "The color of the clockhands.";
    inside_color_area.appendChild(inside_color_label);
    inside_color_area.appendChild(inside_color_picker);
    inside_color_area.appendChild(document.createElement('br'));
    inside_color_area.appendChild(inside_color_description);
    
    let outside_color_area = document.createElement("li");
    let outside_color_label = document.createElement("label");
    let outside_color_description = document.createElement("p");
    outside_color_label.innerHTML = "Frame Color: ";
    outside_color_description.innerHTML = "The color of the frame. Only the chrominance value will be used, the luminance will be ignored.";
    outside_color_area.appendChild(outside_color_label);
    outside_color_area.appendChild(outside_color_picker);
    outside_color_area.appendChild(document.createElement('br'));
    outside_color_area.appendChild(outside_color_description);

    let dot_quantity_area = document.createElement('li');
    let dot_quantity_label = document.createElement('label');
    //let dot_quantity_description = document.createElement('p');
    dot_quantity_label.innerHTML = "Number of Dots tracing the frame: ";
    dot_quantity_area.appendChild(dot_quantity_label);
    dot_quantity_area.appendChild(dot_quantity_picker);

    let time_scale_area = document.createElement('li');
    let time_scale_label = document.createElement('label');
    let time_scale_description = document.createElement("p");
    time_scale_label.innerHTML = "Dot speed: ";
    time_scale_description.innerHTML = "Lower is faster. This is the time in seconds for a dot to go all the way around the frame.";
    time_scale_area.appendChild(time_scale_label);
    time_scale_area.appendChild(time_scale_picker);
    time_scale_area.appendChild(document.createElement('br'));
    time_scale_area.appendChild(time_scale_description);

    let decay_rate_area = document.createElement('li');
    let decay_rate_label = document.createElement('label');
    let decay_rate_description = document.createElement('p');
    decay_rate_label.innerHTML = "Trail decay rate: ";
    decay_rate_description.innerHTML = "How quickly the trail behind the frame decays. Intended to go from 0 to 1, but if you want the next dot to arrive before the trail fully disappears, you can choose a number in the range -infinity to 0.";
    decay_rate_area.appendChild(decay_rate_label);
    decay_rate_area.appendChild(decay_rate_picker);
    decay_rate_area.appendChild(document.createElement('br'));
    decay_rate_area.appendChild(decay_rate_description);

    let submit_button = document.createElement('button');
    submit_button.innerHTML = "Submit";
    submit_button.onclick = () => {
        const hostname = "ledtest.local";
        const port_number = 9002;
        let sock = new WebSocket(`ws://${hostname}:${port_number}`);
        sock.onopen = () => {
            let output_string = `NEW\nclock_mode\n-d\n${decay_rate_picker.value}\n-n\n${dot_quantity_picker.value}\n-t\n${time_scale_picker.value}\n-i\n${inside_color_picker.value.slice(1)}\n-o\n${outside_color_picker.value.slice(1)}`;
            sock.send(output_string);
            sock.close();
        }
        
    }

    var_list.appendChild(inside_color_area);
    var_list.appendChild(outside_color_area);
    var_list.appendChild(dot_quantity_area);
    var_list.appendChild(time_scale_area);
    var_list.appendChild(decay_rate_area);

    dumpingground.appendChild(var_list);
    dumpingground.appendChild(submit_button);
}