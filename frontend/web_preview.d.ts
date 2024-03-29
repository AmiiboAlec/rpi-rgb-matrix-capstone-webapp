export declare class color {
    r: number;
    g: number;
    b: number;
    constructor(red: number, green: number, blue: number);
    toString(): string;
}
export declare function get_color_from_color_input(input_element: HTMLInputElement): color;
export declare class preview {
    private dot_copier;
    protected canvas: HTMLCanvasElement;
    private ctx;
    private framebuffer;
    protected width: number;
    protected height: number;
    private onclick_function;
    protected preview_mode: boolean;
    protected sock: WebSocket | null;
    constructor(canvas: HTMLCanvasElement, width: number, height: number);
    update_pixel(x: number, y: number, color: color): void;
    set_onclick_event_function(callback: (x: number, y: number) => void): void;
    private redraw_pixel;
    private draw_background;
    private redraw;
    enable_preview(): void;
    disable_preview(): void;
    resize_canvas(): void;
}
