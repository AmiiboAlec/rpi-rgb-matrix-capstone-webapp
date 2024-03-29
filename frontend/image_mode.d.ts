import { color, preview } from "./web_preview.js";
export declare class sequence extends preview {
    private submit_button;
    private delay;
    private frame_list;
    private frame_index;
    private selected_color;
    private upload_button;
    live_preview_button: HTMLInputElement;
    live_preview_label: HTMLLabelElement;
    constructor(width: number, height: number, preview_area: HTMLDivElement);
    set_delay(delay: number): void;
    handle_mouse(event: MouseEvent): void;
    toString(): string;
    private draw_frame;
    get_pixel(x: number, y: number): color;
    private set_pixel;
    draw_next_frame(): void;
    draw_previous_frame(): void;
    submit(): void;
    new_framelist(imgs: Array<HTMLImageElement>): void;
    new_image(img: HTMLImageElement): void;
    get_upload_button(): HTMLInputElement;
}
