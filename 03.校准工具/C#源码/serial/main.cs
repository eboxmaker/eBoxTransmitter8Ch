using System;
using System.IO.Ports;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;

namespace serial
{
    public partial class main : Form
    {
        SerialPort sp1 = new SerialPort();
        DDC ddc = new DDC();
        DataTable dtpt100 = new DataTable();
        DataTable dtrx = new DataTable();

        public main()
        {
            InitializeComponent();
        }

        private void main_Load(object sender, EventArgs e)
        {
            //检查是否含有串口
  
            string[] str = SerialPort.GetPortNames();
            if (str == null)
            {
                MessageBox.Show("本机没有串口", "Error");
                return;
            }
            cbBaudRate.Items.Add("9600");
            cbBaudRate.Items.Add("115200");
            cbBaudRate.SelectedIndex = 1;
            cbData.Items.Add("5");
            cbData.Items.Add("6");
            cbData.Items.Add("7");
            cbData.Items.Add("8");
            cbData.SelectedIndex = 3;
            cbStop.Items.Add("1");
            cbStop.Items.Add("1.5");
            cbStop.Items.Add("1.2");
            cbStop.SelectedIndex = 0;
            cbParity.Items.Add("无");
            cbParity.Items.Add("奇校验");
            cbParity.Items.Add("偶校验");
            cbParity.SelectedIndex = 0;
            //添加串口
            foreach (string s in System.IO.Ports.SerialPort.GetPortNames())
            {
                cbSerial.Items.Add(s);
            }
            try
            {
                cbSerial.SelectedIndex = 0;//必须有串口
                sp1.BaudRate = 115200;
            }
            catch (System.Exception ex)
            {
                MessageBox.Show("Error:" + ex.Message, "Error");

                //tmSend.Enabled = false;
                return;
            }
            Control.CheckForIllegalCrossThreadCalls = false;
            sp1.DataReceived += new SerialDataReceivedEventHandler(sp1_DataReceived);

            btnSendAdjustPt100.Enabled = false;
            btnSendAdjustRx.Enabled = false;
            btnExitCal.Enabled = false;
            btnEnterCal.Enabled = false;

            dtpt100.Columns.Add("ADC");
            dtpt100.Columns.Add("VALUE");

            dtrx.Columns.Add("ADC");
            dtrx.Columns.Add("VALUE");
            dgw1.SelectionMode = DataGridViewSelectionMode.FullRowSelect;
            dgw2.SelectionMode = DataGridViewSelectionMode.FullRowSelect;

            cbPTMode.Items.Add("PT100");
            cbPTMode.Items.Add("PT1000");
            cbPTMode.SelectedIndex = 0;
            cbPTMode.Enabled = false;

        }
        void sp1_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {

            if (sp1.IsOpen)
            {
               // try
                {

                    int byteToRead = sp1.BytesToRead;
                    Byte[] receiveData = new Byte[byteToRead];
                    sp1.Read(receiveData, 0, byteToRead);
                    sp1.DiscardInBuffer();
                    byte[] strRcv = new byte[byteToRead];
                    //string strRcv1 = Encoding.Default.GetString(receiveData);

                    
                    string strRcvChar = null;
                    for (int i = 0; i < byteToRead; i++)
                    {
                        strRcvChar += (char)receiveData[i];
                    }
                    for (int i = 0; i < byteToRead; i++)
                    {
                        ddc.recvProcess(receiveData[i]);
                        if (ddc.flag == true)
                        {
                            tbRDDChead.Text =BitConverter.ToUInt16( ddc.rFrame.head,0).ToString();
                            tbRDDCid.Text = ddc.rFrame.id.ToString();
                            tbRDDCch.Text = ddc.rFrame.ch.ToString();
                            tbRDDCack.Text = ddc.rFrame.ack.ToString();
                            tbRDDCpaylen.Text = ddc.rFrame.payLen.ToString();
                            tbRDDCpay.Text = System.Text.Encoding.Default.GetString(ddc.rFrame.pay);
                            tbRDDCcrc.Text = ddc.rFrame.crc.ToString();

                            switch(ddc.rFrame.ch)
                            {
                                case 1:
                                    tbDDCadc0.Text = BitConverter.ToSingle(ddc.rFrame.pay, 0).ToString("F4");
                                    tbDDCvoltage0.Text = BitConverter.ToSingle(ddc.rFrame.pay, 4).ToString("F4");
                                    break;
                                case 2:
                                    tbDDCrxadc.Text = BitConverter.ToSingle(ddc.rFrame.pay, 0).ToString("F4");
                                    tbDDCadc1.Text = BitConverter.ToSingle(ddc.rFrame.pay, 0).ToString("F4");
                                    tbDDCvoltage1.Text = BitConverter.ToSingle(ddc.rFrame.pay, 4).ToString("F4");
                                    break;
                                case 3:
                                    tbRDDCptadc.Text = BitConverter.ToSingle(ddc.rFrame.pay, 0).ToString("F4");
                                    tbDDCadc2.Text = BitConverter.ToSingle(ddc.rFrame.pay, 0).ToString("F4");
                                    tbDDCvoltage2.Text = BitConverter.ToSingle(ddc.rFrame.pay, 4).ToString("F4");
                                    break;
                                case 4:
                                    tbDDCadc3.Text = BitConverter.ToSingle(ddc.rFrame.pay, 0).ToString("F4");
                                    tbDDCvoltage3.Text = BitConverter.ToSingle(ddc.rFrame.pay, 4).ToString("F4");
                                    break;
                                case 5:
                                    tbDDCgetRxValue.Text = BitConverter.ToSingle(ddc.rFrame.pay, 0).ToString("F4");
                                    break;
                                case 6:
                                    tbDDCgetRptValue.Text = BitConverter.ToSingle(ddc.rFrame.pay, 0).ToString("F4");
                                    tbDDCgetTempValueAdjust.Text = BitConverter.ToSingle(ddc.rFrame.pay, 4).ToString("F4");
                                    break;

                                    
                                case 10:
                                    tbDDCcc.Text = BitConverter.ToDouble(ddc.rFrame.pay, 0).ToString("F6");
                                    break;
                                case 11:
                                    tbDDCformula.Text = "y=" + BitConverter.ToDouble(ddc.rFrame.pay, 0).ToString("F6") + "x+ " + BitConverter.ToDouble(ddc.rFrame.pay, 8).ToString("F6");
                                   break;
                                default :
                                    break;

                            }

                            string strRcvHex = null;
                            byte[] frameBuf = ddc.rFrame.frameToBuf(ddc.rFrame.payLen);
                            for (int j = 0; j < ddc.rFrame.payLen + 10; j++)
                            {
                                strRcvHex += " 0x";
                                strRcvHex += frameBuf[j].ToString("X02");
                            }
                            txtRecvHex.Text += "<"+ strRcvHex + ">\r\n";

                            ddc.flag = false;
                        }                   
                    }

                   
                    txtRecvChar.Text += strRcvChar;

                }
               // catch (System.Exception ex)
                {
                //    MessageBox.Show(ex.Message, "出错提示");
                    
                }
            }
        }

        private void btnSwitch_Click(object sender, EventArgs e)
        {
            if (!sp1.IsOpen)
            {
                try
                {
                    string serialName = cbSerial.SelectedItem.ToString();
                    sp1.PortName = serialName;
                    //串口的设置
                    string strBaudRate = cbBaudRate.Text;
                    string strDataBits = cbData.Text;
                    string strStopBits = cbStop.Text;
                    Int32 iBaudRate = Convert.ToInt32(strBaudRate);
                    Int32 iDataBits = Convert.ToInt32(strDataBits);
                    sp1.BaudRate = iBaudRate;
                    sp1.DataBits = iDataBits;
                    switch (cbStop.Text)
                    {
                        case "1":
                            sp1.StopBits = StopBits.One;
                            break;
                        case "1.5":
                            sp1.StopBits = StopBits.OnePointFive;
                            break;
                        case "2":
                            sp1.StopBits = StopBits.Two;
                            break;
                        default:
                            MessageBox.Show("Error:参数不正确", "Error");
                            break;
                    }
                    switch (cbParity.Text)
                    {
                        case "无":
                            sp1.Parity = Parity.None;
                            break;
                        case "奇校验":
                            sp1.Parity = Parity.Odd;
                            break;
                        case "偶校验":
                            sp1.Parity = Parity.Even;
                            break;
                        default:
                            MessageBox.Show("Error：参数不正确!", "Error");
                            break;
                    }
                    if (sp1.IsOpen == true)//如果打开状态，则先关闭一下
                    {
                        sp1.Close();
                    }
                    cbSerial.Enabled = false;
                    sp1.Open();
                    btnSwitch.Text = "关闭串口";
                    //btnSendAdjustPt100.Enabled = true;
                    //btnSendAdjustRx.Enabled = true;
                    //btnExitCal.Enabled = true;
                    btnEnterCal.Enabled = true;
                    cbPTMode.Enabled = true;

                }
                catch (System.Exception ex)
                {
                    MessageBox.Show("Error:" + ex.Message, "Error");
   
                    //tmSend.Enabled = false;
                    return;
                }
            }
            else
            {
                cbSerial.Enabled = true;
                btnSendAdjustPt100.Enabled = false;
                btnSendAdjustRx.Enabled = false;
                btnExitCal.Enabled = false;
                btnEnterCal.Enabled = false;
                cbPTMode.Enabled = false;
                sp1.Close();
                btnSwitch.Text = "打开串口";

            }
        }

        private void btnExit_Click(object sender, EventArgs e)
        {
            sp1.Close();
            Application.Exit();
        }

        private void tabPage2_Click(object sender, EventArgs e)
        {

        }

        private void btnClear_Click(object sender, EventArgs e)
        {
            txtRecvChar.Clear();
            txtRecvHex.Clear();
        }



        private void txtRecv_TextChanged(object sender, EventArgs e)
        {
            //txtRecvChar.Focus();
            this.txtRecvChar.Select(this.txtRecvChar.TextLength, 0);
            this.txtRecvChar.ScrollToCaret();

        }

        private void txtRecvHex_TextChanged(object sender, EventArgs e)
        {
            //txtRecvHex.Focus();
            this.txtRecvHex.Select(this.txtRecvHex.TextLength, 0);
            this.txtRecvHex.ScrollToCaret();
        }


 


        private void dgw_RowStateChanged(object sender, DataGridViewRowStateChangedEventArgs e)
        {
            //显示在HeaderCell上
            for (int i = 0; i < this.dgw1.Rows.Count; i++)
            {
                DataGridViewRow r = this.dgw1.Rows[i];
                r.HeaderCell.Value = string.Format("{0}", i + 1);
            }
            this.dgw1.Refresh();
        }
        private void dgw2_RowStateChanged(object sender, DataGridViewRowStateChangedEventArgs e)
        {
            //显示在HeaderCell上
            for (int i = 0; i < this.dgw2.Rows.Count; i++)
            {
                DataGridViewRow r = this.dgw2.Rows[i];
                r.HeaderCell.Value = string.Format("{0}", i + 1);
            }
            this.dgw2.Refresh();
        }

        private void btnAddAdjustData_Click(object sender, EventArgs e)
        {
            DataRow dr = dtpt100.NewRow();
            dr[0] = tbRDDCptadc.Text.ToString();
            dr[1] = tbDDCpt100value.Text.ToString();
            if (dr[0].ToString() == "" || dr[1].ToString() == "")
                MessageBox.Show("err", "err");
            else
            {

                dtpt100.Rows.Add(dr);
                dgw1.DataSource = dtpt100;
            }

        }

        private void btnSendAdjustData_Click(object sender, EventArgs e)
        {
            double adc;
            double value;
            if (dtpt100.Rows.Count == 0)
            {
                MessageBox.Show("PT100的数据少于2行", "err");
            }
            else
            {
                UInt16 frameLen = (UInt16)(dtpt100.Rows.Count * 16 + 10);
                UInt16 payLen = (UInt16)(dtpt100.Rows.Count * 16);
                byte[] data = new byte[payLen];
                byte[] frame = new byte[frameLen];
                for (int i = 0; i < dtpt100.Rows.Count; i++)
                {
                    adc = Convert.ToDouble(dtpt100.Rows[i][0]);
                    value = Convert.ToDouble(dtpt100.Rows[i][1]);
                    BitConverter.GetBytes(adc).CopyTo(data, i * 16);
                    BitConverter.GetBytes(value).CopyTo(data, i * 16 + 8);
                }
                frame = ddc.makeFrame(data, payLen, 0, 1);
                sp1.Write(frame, 0, frameLen);

            }

        }


        private void btnAddAdjustRxoffset_Click(object sender, EventArgs e)
        {
            DataRow dr = dtrx.NewRow();

            dr[0] = tbDDCrxadc.Text.ToString();
            dr[1] = tbDDCrxvalue.Text.ToString();
            if (dr[0] == "" || dr[1] == "")
                MessageBox.Show("err", "err");
            else
            {

                dtrx.Rows.Add(dr);
                dgw2.DataSource = dtrx;
            }
        }
        private void btnAdjustRx_Click(object sender, EventArgs e)
        {
            double adc;
            double value;
            if(dtrx.Rows.Count == 0)
            {
                MessageBox.Show("线电阻的校准数据少于2行", "err");
            }
            else if (dtrx.Rows.Count == 1)
            {
                UInt16 frameLen = (UInt16)(dtrx.Rows.Count * 16 + 10);
                UInt16 payLen = (UInt16)(dtrx.Rows.Count * 16);
                byte[] data = new byte[payLen];
                byte[] frame = new byte[frameLen];
                adc = Convert.ToDouble(dtrx.Rows[0][0]);
                value = Convert.ToDouble(dtrx.Rows[0][1]);
                BitConverter.GetBytes(adc).CopyTo(data, 0);
                BitConverter.GetBytes(value).CopyTo(data, 8);

                frame = ddc.makeFrame(data, payLen, 0, 2);
                sp1.Write(frame, 0, frameLen);

            }
            else
            {
                UInt16 frameLen = (UInt16)(dtrx.Rows.Count * 16 + 10);
                UInt16 payLen = (UInt16)(dtrx.Rows.Count * 16);
                byte[] data = new byte[payLen];
                byte[] frame = new byte[frameLen];
                for (int i = 0; i < dtrx.Rows.Count; i++)
                {
                    adc = Convert.ToDouble(dtrx.Rows[i][0]);
                    value = Convert.ToDouble(dtrx.Rows[i][1]);
                    BitConverter.GetBytes(adc).CopyTo(data, i * 16);
                    BitConverter.GetBytes(value).CopyTo(data, i * 16 + 8);
                }
                frame = ddc.makeFrame(data, payLen, 0, 2);
                sp1.Write(frame, 0, frameLen);
            }

        }

        private void btnDelPT100SelectRow_Click(object sender, EventArgs e)
        {
            if (dgw1.Rows.Count > 1)
            {
                for (int i = 0; i < dgw1.Rows.Count;i++ )
                {
                    if (dgw1.Rows[i].Selected == true)
                        dgw1.Rows.Remove(dgw1.CurrentRow);

                }
                dgw1.Refresh();
            }

        }

        private void btnDelRxSelectRow_Click(object sender, EventArgs e)
        {
            if (dgw2.Rows.Count > 1)
            {
                for (int i = 0; i < dgw2.Rows.Count; i++)
                {
                    if (dgw2.Rows[i].Selected == true)
                        dgw2.Rows.Remove(dgw2.CurrentRow);

                }
                dgw2.Refresh();
            }
        }

        private void tbSCmd_KeyPress(object sender, KeyPressEventArgs e)
        {

        }

        private void tbSAddr_KeyPress(object sender, KeyPressEventArgs e)
        {

        }

 

        private void btnExport_Click(object sender, EventArgs e)
        {



            string strAdc;
            string strValue;

            string strText = "";
            if (dtrx.Rows.Count == 0 || dtpt100.Rows.Count == 0)
            {
                MessageBox.Show("err", "err");
            }
            else
            {
                strText += "Rx:\r\n";

                for (int i = 0; i < dtrx.Rows.Count; i++)
                {
                    strAdc = dtrx.Rows[i][0].ToString();
                    strValue = dtrx.Rows[i][1].ToString();
                    strText +=strAdc;
                    strText += ",";
                    strText +=strValue;
                    strText += "\r\n";
                }
                strText += "Pt:\r\n";

                for (int i = 0; i < dtpt100.Rows.Count; i++)
                {
                    strAdc = dtpt100.Rows[i][0].ToString();
                    strValue = dtpt100.Rows[i][1].ToString();
                    strText += strAdc;
                    strText += ",";
                    strText += strValue;
                    strText += "\r\n";
                }
                strText += "END\r\n";
                saveText(strText);
            }


        }

        private void saveText(string text)
        {
            string oriStr = text.ToString();
            //string strArr = oriStr.Replace(this.textBox1.Text, string.Format("\r\n{0}", this.textBox1.Text));
            //strArr = strArr.Remove(0, 2);

            SaveFileDialog saveFileDialog1 = new SaveFileDialog();

            saveFileDialog1.Filter = "文本文件(*.txt)|*.txt";
            if (saveFileDialog1.ShowDialog() == DialogResult.OK)
            {
                //使用“另存为”对话框中输入的文件名实例化StreamWriter对象
                StreamWriter sw = new StreamWriter(saveFileDialog1.FileName, true);
                //向创建的文件中写入内容
                sw.Write(oriStr);
                //关闭当前文件写入流
                sw.Close();
            }
        }
        public string getFilePath()
        {
            OpenFileDialog openFileDialog = new OpenFileDialog();
            openFileDialog.InitialDirectory = "c:\\";//注意这里写路径时要用c:\\而不是c:\
            openFileDialog.Filter = "文本文件(*.txt)|*.txt";
            openFileDialog.RestoreDirectory = true;
            openFileDialog.FilterIndex = 1;

            if (openFileDialog.ShowDialog() == DialogResult.OK)
            {
                return openFileDialog.FileName;
                //File fileOpen = new File(fName);
                //isFileHaveName = true;
                //richTextBox1.Text = fileOpen.ReadFile();
                //richTextBox1.AppendText("");
            }
            else
                return null;
        }

        private void btnImport_Click(object sender, EventArgs e)
        {
            string path;
            string str;
            string[] sArray;

            path = getFilePath();
            if (path == null) return;
            StreamReader  sr = new StreamReader(path,Encoding.Default);

            // while ((str = sr.ReadLine()) != null)
                str = sr.ReadLine();
            if (str == "Rx:")
            {
                while((str = sr.ReadLine()) != "Pt:")
                {
                    sArray = str.Split(',');


                    DataRow dr = dtrx.NewRow();
                    dr[0] = sArray[0].ToString();
                    dr[1] = sArray[1].ToString();
                    dtrx.Rows.Add(dr);
                    dgw2.DataSource = dtrx;
                }

                while ((str = sr.ReadLine()) != "END")
                {
                    sArray = str.Split(',');


                    DataRow dr = dtpt100.NewRow();
                    dr[0] = sArray[0].ToString();
                    dr[1] = sArray[1].ToString();
                    dtpt100.Rows.Add(dr);
                    dgw1.DataSource = dtpt100;
                }
    
            }
            else if (str == "Pt")
            { 
            
            }
        }

        private void cbSerial_MouseClick(object sender, MouseEventArgs e)
        {
            int lastSelected = cbSerial.SelectedIndex;

                cbSerial.Items.Clear();
           
            foreach (string s in System.IO.Ports.SerialPort.GetPortNames())
            {
                cbSerial.Items.Add(s);
            }
            cbSerial.SelectedIndex = lastSelected;
        }

        private void tbDDCgetTempValueAdjust_TextChanged(object sender, EventArgs e)
        {

        }
        public static void Delay(int milliSecond)
        {
            int start = Environment.TickCount;
            while (Math.Abs(Environment.TickCount - start) < milliSecond)
            {
                Application.DoEvents();
            }
        }
        private void btnEnterCal_Click(object sender, EventArgs e)
        {
            int i=0;
            byte[] frame = new byte[14];

            i = 0;
            if (cbPTMode.Text == "PT100")
            {

                frame[i++] = 0x01;
                frame[i++] = 0x06;
                frame[i++] = 0x00;
                frame[i++] = 0x47;
                frame[i++] = 0x00;
                frame[i++] = 0x01;
                frame[i++] = 0xf8;
                frame[i++] = 0x1f;

            }
            else
            {
                frame[i++] = 0x01;
                frame[i++] = 0x06;
                frame[i++] = 0x00;
                frame[i++] = 0x47;
                frame[i++] = 0x00;
                frame[i++] = 0x02;
                frame[i++] = 0xb8;
                frame[i++] = 0x1e;

            }
            sp1.Write(frame, 0, i);



            Delay(100);


            i = 0;
            frame[i++] = 0x01;
            frame[i++] = 0x06;
            frame[i++] = 0x00;
            frame[i++] = 0x46;
            frame[i++] = 0x00;
            frame[i++] = 0x01;
            frame[i++] = 0xa9;
            frame[i++] = 0xdf;

            sp1.Write(frame, 0, i);
            btnEnterCal.Enabled = false;
            btnExitCal.Enabled = true;
            btnCalAll.Enabled = true;
            btnCalAll.Enabled = true;
            btnSendAdjustRx.Enabled = true;
            btnSendAdjustPt100.Enabled = true;
            cbPTMode.Enabled = false;


        }
        private void btnExitCal_Click(object sender, EventArgs e)
        {
            UInt32 flag = 0x55aa;
            byte[] data = new byte[4];
            byte[] frame = new byte[14];

            BitConverter.GetBytes(flag).CopyTo(data, 0);
            frame = ddc.makeFrame(data, 4, 0, 20);
            sp1.Write(frame, 0, 14);
            btnEnterCal.Enabled = true;
            btnExitCal.Enabled = false;
            btnCalAll.Enabled = false;
            btnSendAdjustRx.Enabled = false;
            btnSendAdjustPt100.Enabled = false;
            cbPTMode.Enabled = true;
        }

        private void btnCalAll_Click(object sender, EventArgs e)
        {

            btnSendAdjustPt100.PerformClick();
            btnSendAdjustRx.PerformClick();

        }

        private void cbPTMode_SelectedIndexChanged(object sender, EventArgs e)
        {

            int i = 0;
            byte[] frame = new byte[14];  
            if(cbPTMode.Text == "PT100")
            {

                frame[i++] = 0x01;
                frame[i++] = 0x06;
                frame[i++] = 0x00;
                frame[i++] = 0x47;
                frame[i++] = 0x00;
                frame[i++] = 0x01;
                frame[i++] = 0xf8;
                frame[i++] = 0x1f;

            }
            else
            {
                frame[i++] = 0x01;
                frame[i++] = 0x06;
                frame[i++] = 0x00;
                frame[i++] = 0x47;
                frame[i++] = 0x00;
                frame[i++] = 0x02;
                frame[i++] = 0xb8;
                frame[i++] = 0x1e;

            }
            if(sp1.IsOpen == true)
                sp1.Write(frame, 0, i);

        }








 










    }
}
