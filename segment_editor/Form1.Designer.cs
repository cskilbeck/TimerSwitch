namespace segment_editor
{
    partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if(disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.seg_F = new System.Windows.Forms.Button();
            this.seg_A = new System.Windows.Forms.Button();
            this.seg_B = new System.Windows.Forms.Button();
            this.seg_G = new System.Windows.Forms.Button();
            this.seg_E = new System.Windows.Forms.Button();
            this.seg_D = new System.Windows.Forms.Button();
            this.seg_C = new System.Windows.Forms.Button();
            this.label_value = new System.Windows.Forms.Label();
            this.listBox1 = new System.Windows.Forms.ListBox();
            this.save_button = new System.Windows.Forms.Button();
            this.saveFileDialog1 = new System.Windows.Forms.SaveFileDialog();
            this.load_button = new System.Windows.Forms.Button();
            this.openFileDialog1 = new System.Windows.Forms.OpenFileDialog();
            this.SuspendLayout();
            // 
            // seg_F
            // 
            this.seg_F.BackColor = System.Drawing.SystemColors.ActiveCaptionText;
            this.seg_F.FlatAppearance.BorderSize = 0;
            this.seg_F.FlatAppearance.CheckedBackColor = System.Drawing.Color.Red;
            this.seg_F.FlatAppearance.MouseDownBackColor = System.Drawing.Color.Black;
            this.seg_F.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.seg_F.Location = new System.Drawing.Point(75, 71);
            this.seg_F.Margin = new System.Windows.Forms.Padding(0);
            this.seg_F.Name = "seg_F";
            this.seg_F.Size = new System.Drawing.Size(30, 100);
            this.seg_F.TabIndex = 0;
            this.seg_F.TabStop = false;
            this.seg_F.Tag = "5";
            this.seg_F.UseVisualStyleBackColor = true;
            this.seg_F.Click += new System.EventHandler(this.seg_Click);
            // 
            // seg_A
            // 
            this.seg_A.BackColor = System.Drawing.SystemColors.ActiveCaptionText;
            this.seg_A.FlatAppearance.BorderSize = 0;
            this.seg_A.FlatAppearance.CheckedBackColor = System.Drawing.Color.Red;
            this.seg_A.FlatAppearance.MouseDownBackColor = System.Drawing.Color.Black;
            this.seg_A.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.seg_A.Location = new System.Drawing.Point(105, 41);
            this.seg_A.Margin = new System.Windows.Forms.Padding(0);
            this.seg_A.Name = "seg_A";
            this.seg_A.Size = new System.Drawing.Size(100, 30);
            this.seg_A.TabIndex = 1;
            this.seg_A.TabStop = false;
            this.seg_A.Tag = "0";
            this.seg_A.UseVisualStyleBackColor = true;
            this.seg_A.Click += new System.EventHandler(this.seg_Click);
            // 
            // seg_B
            // 
            this.seg_B.BackColor = System.Drawing.SystemColors.ActiveCaptionText;
            this.seg_B.FlatAppearance.BorderSize = 0;
            this.seg_B.FlatAppearance.CheckedBackColor = System.Drawing.Color.Red;
            this.seg_B.FlatAppearance.MouseDownBackColor = System.Drawing.Color.Black;
            this.seg_B.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.seg_B.Location = new System.Drawing.Point(205, 71);
            this.seg_B.Margin = new System.Windows.Forms.Padding(0);
            this.seg_B.Name = "seg_B";
            this.seg_B.Size = new System.Drawing.Size(30, 100);
            this.seg_B.TabIndex = 2;
            this.seg_B.TabStop = false;
            this.seg_B.Tag = "1";
            this.seg_B.UseVisualStyleBackColor = true;
            this.seg_B.Click += new System.EventHandler(this.seg_Click);
            // 
            // seg_G
            // 
            this.seg_G.BackColor = System.Drawing.SystemColors.ActiveCaptionText;
            this.seg_G.FlatAppearance.BorderSize = 0;
            this.seg_G.FlatAppearance.CheckedBackColor = System.Drawing.Color.Red;
            this.seg_G.FlatAppearance.MouseDownBackColor = System.Drawing.Color.Black;
            this.seg_G.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.seg_G.Location = new System.Drawing.Point(105, 171);
            this.seg_G.Margin = new System.Windows.Forms.Padding(0);
            this.seg_G.Name = "seg_G";
            this.seg_G.Size = new System.Drawing.Size(100, 30);
            this.seg_G.TabIndex = 3;
            this.seg_G.TabStop = false;
            this.seg_G.Tag = "6";
            this.seg_G.UseVisualStyleBackColor = true;
            this.seg_G.Click += new System.EventHandler(this.seg_Click);
            // 
            // seg_E
            // 
            this.seg_E.BackColor = System.Drawing.SystemColors.ActiveCaptionText;
            this.seg_E.FlatAppearance.BorderSize = 0;
            this.seg_E.FlatAppearance.CheckedBackColor = System.Drawing.Color.Red;
            this.seg_E.FlatAppearance.MouseDownBackColor = System.Drawing.Color.Black;
            this.seg_E.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.seg_E.Location = new System.Drawing.Point(75, 201);
            this.seg_E.Margin = new System.Windows.Forms.Padding(0);
            this.seg_E.Name = "seg_E";
            this.seg_E.Size = new System.Drawing.Size(30, 100);
            this.seg_E.TabIndex = 4;
            this.seg_E.TabStop = false;
            this.seg_E.Tag = "4";
            this.seg_E.UseVisualStyleBackColor = true;
            this.seg_E.Click += new System.EventHandler(this.seg_Click);
            // 
            // seg_D
            // 
            this.seg_D.BackColor = System.Drawing.SystemColors.ActiveCaptionText;
            this.seg_D.FlatAppearance.BorderSize = 0;
            this.seg_D.FlatAppearance.CheckedBackColor = System.Drawing.Color.Red;
            this.seg_D.FlatAppearance.MouseDownBackColor = System.Drawing.Color.Black;
            this.seg_D.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.seg_D.Location = new System.Drawing.Point(105, 301);
            this.seg_D.Margin = new System.Windows.Forms.Padding(0);
            this.seg_D.Name = "seg_D";
            this.seg_D.Size = new System.Drawing.Size(100, 30);
            this.seg_D.TabIndex = 5;
            this.seg_D.TabStop = false;
            this.seg_D.Tag = "3";
            this.seg_D.UseVisualStyleBackColor = true;
            this.seg_D.Click += new System.EventHandler(this.seg_Click);
            // 
            // seg_C
            // 
            this.seg_C.BackColor = System.Drawing.SystemColors.ActiveCaptionText;
            this.seg_C.FlatAppearance.BorderSize = 0;
            this.seg_C.FlatAppearance.CheckedBackColor = System.Drawing.Color.Red;
            this.seg_C.FlatAppearance.MouseDownBackColor = System.Drawing.Color.Black;
            this.seg_C.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.seg_C.Location = new System.Drawing.Point(205, 201);
            this.seg_C.Margin = new System.Windows.Forms.Padding(0);
            this.seg_C.Name = "seg_C";
            this.seg_C.Size = new System.Drawing.Size(30, 100);
            this.seg_C.TabIndex = 6;
            this.seg_C.TabStop = false;
            this.seg_C.Tag = "2";
            this.seg_C.UseVisualStyleBackColor = true;
            this.seg_C.Click += new System.EventHandler(this.seg_Click);
            // 
            // label_value
            // 
            this.label_value.BackColor = System.Drawing.SystemColors.ControlDarkDark;
            this.label_value.Font = new System.Drawing.Font("Consolas", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label_value.ForeColor = System.Drawing.Color.White;
            this.label_value.Location = new System.Drawing.Point(48, 367);
            this.label_value.Name = "label_value";
            this.label_value.Size = new System.Drawing.Size(226, 30);
            this.label_value.TabIndex = 7;
            this.label_value.Text = "0000000 = 0x00";
            this.label_value.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // listBox1
            // 
            this.listBox1.ColumnWidth = 50;
            this.listBox1.Dock = System.Windows.Forms.DockStyle.Right;
            this.listBox1.FormattingEnabled = true;
            this.listBox1.IntegralHeight = false;
            this.listBox1.ItemHeight = 20;
            this.listBox1.Location = new System.Drawing.Point(946, 0);
            this.listBox1.Name = "listBox1";
            this.listBox1.Size = new System.Drawing.Size(120, 1121);
            this.listBox1.TabIndex = 8;
            this.listBox1.SelectedIndexChanged += new System.EventHandler(this.listBox1_SelectedIndexChanged);
            // 
            // save_button
            // 
            this.save_button.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.save_button.AutoSize = true;
            this.save_button.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.save_button.Location = new System.Drawing.Point(12, 1079);
            this.save_button.Name = "save_button";
            this.save_button.Size = new System.Drawing.Size(55, 30);
            this.save_button.TabIndex = 9;
            this.save_button.Text = "Save";
            this.save_button.UseVisualStyleBackColor = true;
            this.save_button.Click += new System.EventHandler(this.Save);
            // 
            // saveFileDialog1
            // 
            this.saveFileDialog1.FileName = "segments.c";
            // 
            // load_button
            // 
            this.load_button.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.load_button.AutoSize = true;
            this.load_button.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.load_button.Location = new System.Drawing.Point(75, 1079);
            this.load_button.Name = "load_button";
            this.load_button.Size = new System.Drawing.Size(55, 30);
            this.load_button.TabIndex = 10;
            this.load_button.Text = "Load";
            this.load_button.UseVisualStyleBackColor = true;
            this.load_button.Click += new System.EventHandler(this.load_button_Click);
            // 
            // openFileDialog1
            // 
            this.openFileDialog1.DefaultExt = "c";
            this.openFileDialog1.FileName = "openFileDialog1";
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(9F, 20F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(32)))), ((int)(((byte)(32)))), ((int)(((byte)(32)))));
            this.ClientSize = new System.Drawing.Size(1066, 1121);
            this.Controls.Add(this.load_button);
            this.Controls.Add(this.save_button);
            this.Controls.Add(this.listBox1);
            this.Controls.Add(this.label_value);
            this.Controls.Add(this.seg_C);
            this.Controls.Add(this.seg_D);
            this.Controls.Add(this.seg_E);
            this.Controls.Add(this.seg_G);
            this.Controls.Add(this.seg_B);
            this.Controls.Add(this.seg_A);
            this.Controls.Add(this.seg_F);
            this.Name = "Form1";
            this.Text = "Form1";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button seg_F;
        private System.Windows.Forms.Button seg_A;
        private System.Windows.Forms.Button seg_B;
        private System.Windows.Forms.Button seg_G;
        private System.Windows.Forms.Button seg_E;
        private System.Windows.Forms.Button seg_D;
        private System.Windows.Forms.Button seg_C;
        private System.Windows.Forms.Label label_value;
        private System.Windows.Forms.ListBox listBox1;
        private System.Windows.Forms.Button save_button;
        private System.Windows.Forms.SaveFileDialog saveFileDialog1;
        private System.Windows.Forms.Button load_button;
        private System.Windows.Forms.OpenFileDialog openFileDialog1;
    }
}

