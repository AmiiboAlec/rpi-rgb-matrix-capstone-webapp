declare class transformation {
    my_area: HTMLDivElement;
    t_type: HTMLSelectElement;
    coef: HTMLInputElement;
    con: HTMLInputElement;
    static options: Map<string, string>;
    constructor(form_area: HTMLDivElement);
    toString: () => string;
}
declare let transformations: Array<transformation>;
