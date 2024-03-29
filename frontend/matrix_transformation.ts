class transformation {
    my_area: HTMLDivElement
    t_type: HTMLSelectElement
    coef: HTMLInputElement
    con: HTMLInputElement
    static options = new Map<string, string>([
        ["Translate X", "tx"],
        ["Translate Y", "ty"],
        ["Shear X", "hx"],
        ["Shear Y", "hy"],
        ["Stretch X", "sx"],
        ["Stretch Y", "sy"],
        ["Rotate", "r"]
    ]);
    constructor(form_area: HTMLDivElement) {
        this.t_type = document.createElement('select');
        this.coef = document.createElement('input');
        this.coef.type = "Number";
        this.con = document.createElement('input');
        this.con.type = "Number";
        let intro_text = document.createElement('p');
        let middle_text = document.createElement('p');
        let final_text = document.createElement('p');
        intro_text.innerHTML = "";
        middle_text.innerHTML = "";
        final_text.innerHTML = "";
        intro_text.style.display = "inline";
        middle_text.style.display = "inline";
        final_text.style.display = "inline";
        this.coef.style.display = "inline";
        this.con.style.display = "inline";
        transformation.options.forEach((value, key) => {
            let type_opt = document.createElement('option');
            type_opt.text = key;
            this.t_type.appendChild(type_opt);
        });
        this.t_type.onchange = () => { 
            let label = this.t_type.item(this.t_type.selectedIndex).text;
            if (label === "Rotate") {
                intro_text.innerHTML = "Rotate by ";
                middle_text.innerHTML = " full rotations per second, starting at "
                final_text.innerHTML = " of a full rotation."
            }
            else {
                intro_text.innerHTML = `${label} by `;
                middle_text.innerHTML = " pixels per second, starting at ";
                final_text.innerHTML = "";
            }
        }
        this.t_type.item(0).selected = true;
        form_area.appendChild(this.t_type);
        form_area.appendChild(document.createElement('br'));
        form_area.appendChild(intro_text);
        form_area.appendChild(this.coef);
        form_area.appendChild(middle_text);
        form_area.appendChild(this.con);
        form_area.appendChild(final_text);
    }

    toString = (): string => {
        let type = this.t_type.item(this.t_type.selectedIndex).text;
        type = transformation.options[type];

        return `${type}${this.coef.value}t+${this.con.value}`;
    }
}

let transformations: Array<transformation>;

window.onload = () => {
    let dumpingground = <HTMLDivElement>document.getElementById("Form div");
    let add_transformation = document.createElement('button');
    let submit_button = document.createElement('button');
    let image_input = document.createElement('input');
    image_input.type = "file";
    dumpingground.appendChild(add_transformation);
    add_transformation.onclick = () => {
        let new_div = document.createElement('div');
        let new_transformation = new transformation(new_div);
        transformations.push(new_transformation);
        dumpingground.insertBefore(new_div, add_transformation);
    }
    submit_button.onclick = () => {
        const port = 9002
        const hostname = "ledtest.local";
        let sock = new WebSocket(`ws://${hostname}:${port}`)
        sock.onopen = () => {
            let message = "NEW\nmatrix_transformation"
        }
    }
};