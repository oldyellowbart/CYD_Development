import os
import tkinter as tk
from tkinter import filedialog, messagebox, ttk
from PIL import Image, ImageEnhance, ImageTk, ImageOps
import logging
from datetime import datetime

class GIFtoHexConverter:
    def __init__(self, root):
        self.root = root
        self.root.title("GIF to Hex Converter")
        self.root.configure(bg="#0d1b2a")  # Dark futuristic background

        # Styling
        style = ttk.Style()
        style.configure("TLabel", foreground="cyan", background="#0d1b2a", font=("Arial", 10, "bold"))
        style.configure("TButton", foreground="black", background="cyan", font=("Arial", 10, "bold"), padding=5)
        style.configure("TEntry", font=("Arial", 10), padding=5)
        style.configure("Horizontal.TScale", background="#1b263b")

        # Input File
        ttk.Label(root, text="Input GIF:").grid(row=0, column=0, padx=5, pady=5)
        self.input_path = ttk.Entry(root, width=40)
        self.input_path.grid(row=0, column=1, padx=5, pady=5)
        ttk.Button(root, text="Browse", command=self.load_input_files).grid(row=0, column=2, padx=5, pady=5)

        # Output File
        ttk.Label(root, text="Output File:").grid(row=1, column=0, padx=5, pady=5)
        self.output_path = ttk.Entry(root, width=40)
        self.output_path.grid(row=1, column=1, padx=5, pady=5)
        ttk.Button(root, text="Browse", command=self.load_output_file).grid(row=1, column=2, padx=5, pady=5)

        # Width & Height
        ttk.Label(root, text="Width:").grid(row=2, column=0, padx=5, pady=5)
        self.width_entry = ttk.Entry(root, width=10)
        self.width_entry.grid(row=2, column=1, padx=5, pady=5, sticky="w")
        self.width_entry.insert(0, "64")
        self.width_entry.bind("<KeyRelease>", lambda e: self.update_preview())

        ttk.Label(root, text="Height:").grid(row=3, column=0, padx=5, pady=5)
        self.height_entry = ttk.Entry(root, width=10)
        self.height_entry.grid(row=3, column=1, padx=5, pady=5, sticky="w")
        self.height_entry.insert(0, "64")
        self.height_entry.bind("<KeyRelease>", lambda e: self.update_preview())

        # Invert Checkbox
        self.invert_var = tk.IntVar()
        ttk.Checkbutton(root, text="Invert Colors", variable=self.invert_var, command=self.update_preview).grid(row=4, column=1, sticky="w")

        # Gamma Correction
        ttk.Label(root, text="Gamma:").grid(row=5, column=0, padx=5, pady=5)
        self.gamma_value = tk.DoubleVar(value=128)
        self.gamma_entry = ttk.Entry(root, width=10, textvariable=self.gamma_value)
        self.gamma_entry.grid(row=5, column=1, padx=5, pady=5, sticky="w")
        self.gamma_entry.bind("<KeyRelease>", lambda e: self.update_preview())

        # Gamma Slider
        self.gamma_slider = ttk.Scale(root, from_=10, to=255, orient="horizontal", variable=self.gamma_value, command=self.update_preview)
        self.gamma_slider.grid(row=5, column=2, padx=5, pady=5, sticky="we")

        # Color Mode Dropdown (row 6)
        ttk.Label(root, text="Color Mode:").grid(row=6, column=0, padx=5, pady=5)
        self.color_mode = tk.StringVar(value="8-bit")
        self.color_mode_dropdown = ttk.Combobox(root, textvariable=self.color_mode, values=["8-bit", "16-bit"])
        self.color_mode_dropdown.grid(row=6, column=1, padx=5, pady=5, sticky="w")
        self.color_mode_dropdown.bind("<<ComboboxSelected>>", lambda e: self.update_preview())

        # RGB Format Dropdown
        ttk.Label(root, text="RGB Format:").grid(row=6, column=2, padx=5, pady=5)
        self.rgb_format = tk.StringVar(value="RGB565")
        self.rgb_format_dropdown = ttk.Combobox(
            root, 
            textvariable=self.rgb_format, 
            values=["RGB565", "RGB565_SWAPPED", "BGR565", "BGR565_SWAPPED"]
        )
        self.rgb_format_dropdown.grid(row=6, column=3, padx=5, pady=5, sticky="w")
        self.rgb_format_dropdown.bind("<<ComboboxSelected>>", lambda e: self.update_preview())

        # Log File Checkbox (add before Frame Delay)
        self.save_log_var = tk.BooleanVar(value=False)
        ttk.Checkbutton(
            root, 
            text="Save Log File", 
            variable=self.save_log_var
        ).grid(row=6, column=4, padx=5, pady=5, sticky="w")

        # Frame Delay (row 7)
        ttk.Label(root, text="Frame Delay (ms):").grid(row=7, column=0, padx=5, pady=5)
        self.delay_entry = ttk.Entry(root, width=10)
        self.delay_entry.grid(row=7, column=1, padx=5, pady=5, sticky="w")
        self.delay_entry.insert(0, "30")

        # Convert Button (moved to row 7, column 2)
        ttk.Button(root, text="Convert", command=self.process_gif).grid(row=7, column=2, pady=10, padx=5)

        # Preview Label (row 8)
        self.preview_label = ttk.Label(root, text="Preview:")
        self.preview_label.grid(row=8, column=0, columnspan=3, pady=5)

        # Canvas for Image Preview (row 9)
        self.canvas = tk.Canvas(root, width=200, height=200, bg="gray")  # Increased size for better visibility
        self.canvas.grid(row=9, column=0, columnspan=3, pady=5)

        # Progress Bar (row 10)
        self.progress_var = tk.DoubleVar()
        self.progress_bar = ttk.Progressbar(root, variable=self.progress_var, maximum=100)
        self.progress_bar.grid(row=10, column=0, columnspan=3, padx=5, pady=5, sticky="ew")

        # Log Display (row 11)
        self.log_text = tk.Text(root, height=5, width=50, bg="#1b263b", fg="cyan")
        self.log_text.grid(row=11, column=0, columnspan=3, padx=5, pady=5)

        self.original_frame = None  # Store original GIF frame

        # Setup logging
        self.setup_logging()

    def setup_logging(self):
        """Configure logging to both file and GUI."""
        if self.save_log_var.get():
            log_file = f"gif_converter_{datetime.now().strftime('%Y%m%d_%H%M%S')}.log"
            logging.basicConfig(
                level=logging.INFO,
                format='%(asctime)s - %(levelname)s - %(message)s',
                handlers=[
                    logging.FileHandler(log_file),
                    logging.StreamHandler()
                ]
            )
        else:
            logging.basicConfig(
                level=logging.INFO,
                format='%(asctime)s - %(levelname)s - %(message)s',
                handlers=[
                    logging.StreamHandler()
                ]
            )
        self.logger = logging.getLogger(__name__)

    def log_message(self, message, level="info"):
        """Add message to log display and file."""
        if level == "info":
            self.logger.info(message)
        elif level == "error":
            self.logger.error(message)
        
        self.log_text.insert(tk.END, f"{datetime.now().strftime('%H:%M:%S')} - {message}\n")
        self.log_text.see(tk.END)
        self.root.update_idletasks()

    def load_input_files(self):
        files = filedialog.askopenfilenames(
            title="Select image files",
            filetypes=[
                ("Image Files", "*.gif;*.jpg;*.jpeg;*.png;*.bmp"),
                ("GIF Files", "*.gif"),
                ("JPEG Files", "*.jpg;*.jpeg"),
                ("PNG Files", "*.png"),
                ("BMP Files", "*.bmp")
            ]
        )
        if files:
            self.input_path.delete(0, tk.END)
            self.input_path.insert(0, ";".join(files))
            self.load_preview(files[0])

    def load_output_file(self):
        file_path = filedialog.asksaveasfilename(defaultextension=".h", filetypes=[("Header Files", "*.h")])
        if file_path:
            self.output_path.delete(0, tk.END)
            self.output_path.insert(0, file_path)

    def load_preview(self, gif_path):
        """Load the first frame of GIF for preview."""
        gif = Image.open(gif_path)
        gif.seek(0)  # First frame
        self.original_frame = gif.convert("RGB")  # Convert to RGB for preview
        self.update_preview()

    def update_preview(self, *_):
        """Update the preview based on user input."""
        if not self.original_frame:
            return
        
        try:
            width = int(self.width_entry.get())
            height = int(self.height_entry.get())
            gamma = self.gamma_value.get()
            invert = bool(self.invert_var.get())
            color_mode = self.color_mode.get()

            img = self.original_frame.resize((width, height))
            enhancer = ImageEnhance.Brightness(img)
            img = enhancer.enhance(gamma / 128.0)
            
            if color_mode == "8-bit":
                img = img.convert('1')
                if invert:
                    img = ImageOps.invert(img)
            elif color_mode == "16-bit":
                img = img.convert('RGB')

            img_tk = ImageTk.PhotoImage(img)
            self.canvas.config(width=width, height=height)
            self.canvas.create_image(width//2, height//2, image=img_tk, anchor=tk.CENTER)
            self.canvas.image = img_tk  

        except ValueError:
            pass  

    def process_gif(self):
        try:
            input_files = self.input_path.get().split(";") 
            output_file = self.output_path.get()
            
            # Get parameters from GUI inputs
            width = int(self.width_entry.get())
            height = int(self.height_entry.get())
            gamma = self.gamma_value.get()
            invert = bool(self.invert_var.get())
            
            if not input_files or not output_file:
                messagebox.showerror("Error", "Please select input and output files.")
                return

            self.log_message("Starting GIF conversion process...")
            
            # Reset progress bar
            self.progress_var.set(0)
            
            # Process each file
            for index, image_path in enumerate(input_files):
                self.generate_bitmap_arrays_from_image(  # Changed from generate_bitmap_arrays_from_gif
                    image_path, 
                    output_file, 
                    width, 
                    height, 
                    gamma, 
                    invert
                )
                
                # Update progress
                progress = ((index + 1) / len(input_files)) * 100
                self.progress_var.set(progress)
                self.root.update_idletasks()

            self.log_message("Conversion completed successfully!")
            messagebox.showinfo("Success", "GIF conversion completed successfully!")
                    
        except Exception as e:
            error_msg = f"An error occurred: {str(e)}"
            self.log_message(error_msg, "error")
            messagebox.showerror("Error", error_msg)

    def gif_to_frames(self, image_path, num_frames=28):
        """Extract frames from GIF or create single frame for static images."""
        img = Image.open(image_path)
        frames = []
        
        # Check if image is animated GIF
        if getattr(img, "is_animated", False):
            for i in range(min(num_frames, img.n_frames)):
                img.seek(i)
                frames.append(img.copy())
        else:
            # For static images, create single frame
            frames.append(img.copy())
        
        return frames

    def image_to_hex_array(self, image, width, height, gamma, invert):
        """Convert image to a hex array."""
        color_mode = self.color_mode.get()
        rgb_format = self.rgb_format.get()
        
        if color_mode == "8-bit":
            img = image.convert('L')
            enhancer = ImageEnhance.Brightness(img)
            img = enhancer.enhance(gamma / 128.0)
            img = img.resize((width, height))
            img = img.convert('1')
            if invert:
                img = ImageOps.invert(img)

            pixel_data = list(img.getdata())
            hex_array = []
            for i in range(0, len(pixel_data), 8):
                byte = 0
                for j in range(8):
                    if i + j < len(pixel_data):
                        byte = (byte << 1) | (1 if pixel_data[i + j] > 128 else 0)
                hex_array.append(f'0x{byte:02x}')
            return ', '.join(hex_array)
        elif color_mode == "16-bit":
            img = image.convert('RGB')
            img = img.resize((width, height))
            pixel_data = list(img.getdata())
            hex_array = []
            
            for r, g, b in pixel_data:
                if rgb_format == "RGB565":
                    rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
                elif rgb_format == "RGB565_SWAPPED":
                    rgb565 = (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))
                    rgb565 = ((rgb565 >> 8) | (rgb565 << 8)) & 0xFFFF
                elif rgb_format == "BGR565":
                    rgb565 = ((b & 0xF8) << 8) | ((g & 0xFC) << 3) | (r >> 3)
                elif rgb_format == "BGR565_SWAPPED":
                    rgb565 = ((b & 0xF8) << 8) | ((g & 0xFC) << 3) | (r >> 3)
                    rgb565 = ((rgb565 >> 8) | (rgb565 << 8)) & 0xFFFF
                
                hex_array.append(f'0x{rgb565:04x}')
            return ', '.join(hex_array)

    def generate_bitmap_arrays_from_image(self, image_path, output_file, width, height, gamma, invert):
        """Generate and save the hex array for both GIFs and static images."""
        base_name = os.path.splitext(os.path.basename(image_path))[0]
        frames = self.gif_to_frames(image_path)
        
        # Check if file exists
        file_exists = os.path.exists(output_file)
        mode = 'a' if file_exists else 'w'
        
        with open(output_file, mode) as f:
            # Write headers only for new file
            if not file_exists:
                f.write("#include <TFT_eSPI.h>\n\n")
                f.write("TFT_eSPI tft = TFT_eSPI();\n\n")
                f.write("void cyd_initialize() {\n")
                f.write("    tft.begin();\n")
                f.write("    tft.setRotation(0);\n")
                f.write("    tft.fillScreen(TFT_BLACK);\n")
                f.write("}\n\n")
            
            f.write(f"\n// Arrays for {base_name}\n")
            # Write array for each frame
            for i, frame in enumerate(frames):
                hex_array = self.image_to_hex_array(frame, width, height, gamma, invert)
                f.write(f"const uint16_t {base_name}_frame{i+1}[] PROGMEM = {{{hex_array}}};\n")
            
            # Write dimensions
            f.write(f"\nconst int {base_name}_Width = {width};\n")
            f.write(f"const int {base_name}_Height = {height};\n\n")
            
            # Write static display function
            f.write(f"void {base_name}_static(int x, int y) {{\n")
            f.write("    tft.fillScreen(TFT_BLACK);\n")
            f.write(f"    tft.pushImage(x, y, {base_name}_Width, {base_name}_Height, {base_name}_frame1);\n")
            f.write("}\n\n")
            
            # Write animation function only if multiple frames exist
            if len(frames) > 1:
                f.write(f"void {base_name}_animated(int x, int y, int fps) {{\n")
                f.write("    tft.fillScreen(TFT_BLACK);\n")
                f.write(f"    for(int i = 0; i < {len(frames)}; i++) {{\n")
                f.write(f"        tft.pushImage(x, y, {base_name}_Width, {base_name}_Height, {base_name}_frame{i+1});\n")
                f.write("        delay(fps);\n")
                f.write("    }\n")
                f.write("}\n\n")

        self.log_message(f"Added {base_name} to {output_file}")

if __name__ == "__main__":
    root = tk.Tk()
    app = GIFtoHexConverter(root)
    root.mainloop()
