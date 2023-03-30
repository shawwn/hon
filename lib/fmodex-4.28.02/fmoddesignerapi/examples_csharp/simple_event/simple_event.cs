/*=======================================================================
  Simple Event Example.  Copyright (c) Firelight Technologies 2004-2007.

 This example plays an event created with the FMOD Designer sound
 designer tool.  It simply plays an event, retrieves the parameters
 and allows the user to adjust them.
=========================================================================*/

using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Data;

namespace simple_event
{
	public class Form1 : System.Windows.Forms.Form
    {
        private System.Windows.Forms.Label label;
        private System.Windows.Forms.Timer timer1;
        private System.Windows.Forms.Button exit_button;
        private System.Windows.Forms.StatusBar statusBar;
        private System.ComponentModel.IContainer components;

        private bool initialised    = false;
        private bool eventstart     = false;
        private bool exit           = false;

        /*
            ALL FMOD CALLS MUST HAPPEN WITHIN THE SAME THREAD.
            WE WILL DO EVERYTHING IN THE TIMER THREAD 
        */
        FMOD.EventSystem    eventsystem    = null;
        FMOD.Event          evt            = null;
        FMOD.RESULT         result;
        private Button start_button;

        private void timer1_Tick(object sender, System.EventArgs e)
        {
            if (!initialised)
            {
                ERRCHECK(result = FMOD.Event_Factory.EventSystem_Create(ref eventsystem));
                ERRCHECK(result = eventsystem.init(64, FMOD.INITFLAGS.NORMAL, (IntPtr)null, FMOD.EVENT_INITFLAGS.NORMAL));
                ERRCHECK(result = eventsystem.setMediaPath("../../../../examples/media/"));
                ERRCHECK(result = eventsystem.load("examples.fev"));
                ERRCHECK(result = eventsystem.getEvent("examples/FeatureDemonstration/Basics/SimpleEvent", FMOD.EVENT_MODE.DEFAULT, ref evt));

                initialised = true;
            }


            /*
                "Main Loop" 
            */
            ERRCHECK(result = eventsystem.update());

            if (eventstart)
            {
                ERRCHECK(evt.start());
            }
            /*
                Clean up and exit 
            */
            if (exit)
            {
                ERRCHECK(result = eventsystem.release());

                Application.Exit();
            }
        }

		public Form1()
		{
			InitializeComponent();
		}

		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if (components != null) 
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
            this.components = new System.ComponentModel.Container();
            this.label = new System.Windows.Forms.Label();
            this.timer1 = new System.Windows.Forms.Timer(this.components);
            this.exit_button = new System.Windows.Forms.Button();
            this.statusBar = new System.Windows.Forms.StatusBar();
            this.start_button = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // label
            // 
            this.label.Location = new System.Drawing.Point(16, 16);
            this.label.Name = "label";
            this.label.Size = new System.Drawing.Size(264, 32);
            this.label.TabIndex = 17;
            this.label.Text = "Copyright (c) Firelight Technologies 2004-2007";
            this.label.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // timer1
            // 
            this.timer1.Enabled = true;
            this.timer1.Interval = 10;
            this.timer1.Tick += new System.EventHandler(this.timer1_Tick);
            // 
            // exit_button
            // 
            this.exit_button.Location = new System.Drawing.Point(112, 120);
            this.exit_button.Name = "exit_button";
            this.exit_button.Size = new System.Drawing.Size(72, 24);
            this.exit_button.TabIndex = 18;
            this.exit_button.Text = "Exit";
            this.exit_button.Click += new System.EventHandler(this.exit_button_Click);
            // 
            // statusBar
            // 
            this.statusBar.Location = new System.Drawing.Point(0, 158);
            this.statusBar.Name = "statusBar";
            this.statusBar.Size = new System.Drawing.Size(292, 24);
            this.statusBar.TabIndex = 19;
            // 
            // start_button
            // 
            this.start_button.Location = new System.Drawing.Point(102, 72);
            this.start_button.Name = "start_button";
            this.start_button.Size = new System.Drawing.Size(98, 24);
            this.start_button.TabIndex = 20;
            this.start_button.Text = "Start Event";
            this.start_button.Click += new System.EventHandler(this.start_button_Click);
            // 
            // Form1
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(292, 182);
            this.Controls.Add(this.start_button);
            this.Controls.Add(this.statusBar);
            this.Controls.Add(this.exit_button);
            this.Controls.Add(this.label);
            this.Name = "Form1";
            this.Text = "Simple Event Example";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.ResumeLayout(false);

        }
		#endregion

		[STAThread]
		static void Main() 
		{
			Application.Run(new Form1());
        }

        private void Form1_Load(object sender, EventArgs e)
        {

        }

        private void exit_button_Click(object sender, System.EventArgs e)
        {
            exit = true;
        }

        private void start_button_Click(object sender, EventArgs e)
        {
            eventstart = true;
        }

        private void ERRCHECK(FMOD.RESULT result)
        {
            if (result != FMOD.RESULT.OK)
            {
                timer1.Stop();
                MessageBox.Show("FMOD error! " + result + " - " + FMOD.Error.String(result));
                Environment.Exit(-1);
            }
        }
	}
}
