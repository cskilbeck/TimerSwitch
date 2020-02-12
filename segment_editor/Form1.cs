using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO;
using System.Globalization;

namespace segment_editor
{
    public partial class Form1: Form
    {
        const string ident_string = "//SEGMENT EDITOR:";
        byte[] bitmaps = new byte[128];
        byte state;
        int current_index = -1;

        Button[] buttons = new Button[7];

        public Form1()
        {
            InitializeComponent();
            buttons[0] = seg_A;
            buttons[1] = seg_B;
            buttons[2] = seg_C;
            buttons[3] = seg_D;
            buttons[4] = seg_E;
            buttons[5] = seg_F;
            buttons[6] = seg_G;
        }

        void update_segments()
        {
            byte s = state;
            for(int i = 0; i<7; ++i)
            {
                buttons[i].BackColor = ((s & 1) != 0) ? Color.Red : Color.Black;
                s >>= 1;
            }
        }

        private void seg_Click(object sender, EventArgs e)
        {
            Button b = sender as Button;
            string tag = b.Tag as string;
            int id = int.Parse(tag);
            byte bit = (byte)(1u << id);
            state ^= bit;
            bool on = (state & bit) == 0;
            Color c = on ? Color.Black : Color.Red;
            b.BackColor = c;
            StringBuilder value_string = new StringBuilder();
            for(int i = 0; i<7; ++i)
            {
                bool this_bit = (state & (byte)(1u << i)) != 0;
                string bit_str = this_bit ? "1" : "0";
                value_string.Append($"{bit_str}");
            }
            value_string.Append($" = 0x{state:x2}");
            label_value.Text = value_string.ToString();
            if(current_index >= 0)
            {
                bitmaps[current_index] = state;
            }
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            for(int i = 0; i < 128; ++i)
            {
                listBox1.Items.Add(new ListBoxItem(i));
            }
        }

        private void listBox1_SelectedIndexChanged(object sender, EventArgs e)
        {
            ListBoxItem item = (sender as ListBox).SelectedItem as ListBoxItem;
            current_index = item.v;
            state = bitmaps[item.v];
            update_segments();
        }

        private void output_bitmaps(StreamWriter f, string header, string sep1, string sep2, string sep3, string tail, int entries_per_line)
        {
            f.Write(header);
            string sep = sep1;
            for(int i = 0; i<128; ++i)
            {
                f.Write($"{sep}{bitmaps[i]:x2}");
                if(((i + 1) % entries_per_line) == 0)
                {
                    sep = sep2;
                }
                else
                {
                    sep = sep3;
                }
            }
            f.WriteLine(tail);
        }

        private void Save(object sender, EventArgs e)
        {
            if(saveFileDialog1.ShowDialog() == DialogResult.OK)
            {
                try
                {
                    using(StreamWriter f = new StreamWriter(saveFileDialog1.FileName))
                    {
                        output_bitmaps(f, ident_string, "", "", "", "", 128);
                        output_bitmaps(f, "unsigned char segments[128] = {", "\n\t0x", ",\n\t0x", ",0x", "\n};", 16);
                    }
                }
                catch(IOException err)
                {
                    MessageBox.Show($"{err.Message}", "Save File", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
        }

        private void load_button_Click(object sender, EventArgs e)
        {
            if(openFileDialog1.ShowDialog() == DialogResult.OK)
            {
                try
                {
                    string filename = openFileDialog1.FileName;
                    using(StreamReader f = new StreamReader(filename))
                    {
                        /*
                        00...
                        */
                        string s = f.ReadLine();
                        string header = s.Substring(0, ident_string.Length);
                        if(header.CompareTo(ident_string) != 0)
                        {
                            MessageBox.Show($"Error loading {filename} - ident not found", "Load File", MessageBoxButtons.OK, MessageBoxIcon.Error);
                        }
                        else
                        {
                            string data = s.Substring(ident_string.Length);
                            if(data.Length != 256)
                            {
                                MessageBox.Show($"Error loading {filename} - data wrong length", "Load File", MessageBoxButtons.OK, MessageBoxIcon.Error);
                            }
                            else
                            {
                                for(int i = 0; i<128; ++i)
                                {
                                    if(int.TryParse(data.Substring(i * 2, 2), System.Globalization.NumberStyles.HexNumber, CultureInfo.InvariantCulture, out int x))
                                    {
                                        bitmaps[i] = (byte)x;
                                    }
                                    else
                                    {
                                        MessageBox.Show($"Error loading {filename} - bad data format at entry {i}", "Load File", MessageBoxButtons.OK, MessageBoxIcon.Error);
                                    }
                                }
                            }
                        }
                    }
                }
                catch(IOException err)
                {
                    MessageBox.Show($"{err.Message}", "Load File", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
        }
    }

    public class ListBoxItem
    {
        public int v;

        public ListBoxItem(int x)
        {
            v = x;
        }

        public override string ToString()
        {
            string s = "";
            if(v >= 32)
            {
                s = ((char)v).ToString();
            }
            return $"{v}\t{s}";
        }
    }
}