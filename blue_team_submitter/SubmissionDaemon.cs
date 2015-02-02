/* file: SubmissionDaemon.cs
 * author: David Weinman
 * date: 12/2/14
 * description: Submission Daemon
 * class file. Creates file event
 * notification handlers which send
 * submissions to the scoring server.
 */

using System;
using System.IO;
using System.Collections;
using System.Diagnostics;
using System.ServiceProcess;
using System.Net.Sockets;

namespace SubmissionDaemon
{
    public class SubmissionDaemon : ServiceBase
    {
        // scoring server information
        const Int32 port = 7890;
        const String scoringServer = "10.0.27.128";

        // maximum submission size in bytes.
        const int MAXSUBMISSIONLENGTH = 3000;

        // number of challenges
        const int num_challs = 6;

        // challenge names
        string[] chall_names = new string[num_challs];

        // challenge file name to id relations
        static Hashtable chall_ids = new Hashtable();

        // array of file watcher objects
        FileSystemWatcher[] filewatchers = new FileSystemWatcher[num_challs];

        //const String submissions_path = "C:\\Users\\blue_team\\Documents\\submissions\\";
        const String submissions_path = "C:\\Users\\Administrator\\Documents\\submissions\\";

        // submission daemon constructor
        public SubmissionDaemon()
        {
            this.ServiceName = "SubmissionDaemon";
            this.CanStop = true;
            this.CanPauseAndContinue = false;
            this.AutoLog = true;
        }

        // this is the function that logs some
        // message while the submission daemon is
        // running.
        private static void LogEvent(String message)
        {
            string eventSource = "Submission File Monitor Service";
            DateTime dt = new DateTime();
            dt = System.DateTime.UtcNow;
            message = dt.ToLocalTime() + ": " + message;

            EventLog.WriteEntry(eventSource, message);
        }

        // this is the function that gets called when
        // a submission file gets created
        private static void OnCreate(object source, FileSystemEventArgs evntargs)
        {
            String currentContent;
            // log the change
            LogEvent(String.Format("File with name {0} was: {1}", evntargs.Name, evntargs.ChangeType.ToString()));
            // then try to read in the file data and send to submission listener
            try
            {
                // first, read the contents of the submitted file
                //LogEvent(String.Format("About to read all text from {0}.", evntargs.FullPath));
                try
                {
                    currentContent = File.ReadAllText(evntargs.FullPath);
                }
                catch (FileNotFoundException e)
                {
                    LogEvent(String.Format("FileNotFoundException: {0}", e));
                    return;
                }
                // and check that it is within our size limitations
                //LogEvent(String.Format("About to check size of {0}.", evntargs.FullPath));
                if (currentContent.Length > MAXSUBMISSIONLENGTH)
                {
                    LogEvent(String.Format("Submission was {0} bytes too long.\nSubmission ignored.",
                        currentContent.Length - MAXSUBMISSIONLENGTH));
                    return;
                }
                // initialize the tcp client to send the submission to
                TcpClient client = new TcpClient(scoringServer, port);
                // make a byte array our of the submission
                Byte[] submission = System.Text.Encoding.ASCII.GetBytes(String.Format("CHALLENGE_SUBMIT: blue{0}\nBEGIN\n{1}\nEND", chall_ids[evntargs.Name].ToString(), currentContent));
                // prepare client stream to send submission to 
                NetworkStream stream = client.GetStream();
                // Log submission event
                LogEvent(String.Format(
                "Submission Content:\n--------------------------\n{0}\n--------------------------\nSending message with file content now.",
                currentContent));
                // send submission via client stream and log event
                stream.Write(submission, 0, submission.Length);
                LogEvent(String.Format("Submission sent.", currentContent));
                // delete file and log deletion event
                File.Delete(evntargs.FullPath);
                LogEvent(String.Format("Deleted {0}.", evntargs.FullPath));
            }
            catch (SocketException e)
            {
                LogEvent(String.Format("SocketException: {0}", e));
            }
        }

        //  this method is called when the challenge file watcher detects an error. 
        private static void OnError(object source, ErrorEventArgs e)
        {
            //  Show that an error has been detected.
            LogEvent("The Challenge File Watcher has detected an error");
            //  Give more information if the error is due to an internal buffer overflow. 
            if (e.GetException().GetType() == typeof(InternalBufferOverflowException))
            {
                //  This can happen if Windows is reporting many file system events quickly  
                //  and internal buffer of the  FileSystemWatcher is not large enough to handle this 
                //  rate of events. The InternalBufferOverflowException error informs the application 
                //  that some of the file system events are being lost.
                LogEvent(("The challenge file watcher experienced an internal buffer overflow: " + e.GetException().Message));
            }
        }

        protected override void OnStart(string[] args)
        {
            base.OnStart(args);
            int i;

            // make challenge names
            chall_names[0] = "ftp_server";
            chall_names[1] = "stan";
            chall_names[2] = "plugin";
            chall_names[3] = "forgot_my_pass";
            chall_names[4] = "compress_stan";
            chall_names[5] = "pro_tools";

            for (i = 0; i < num_challs; i++)
            {
                // try to make the challenge directory, exit if this fails (should never happen)
                try {
                    Directory.CreateDirectory(submissions_path + chall_names[i]);
                } catch (Exception e) {
                    LogEvent(String.Format("Submission Daemon is stopping because it couldn't create directory: {0}", e));
                    this.Stop();
                }
                // make challenge file name relations id
                chall_ids[chall_names[i] + ".txt"] = i + 1;
                // initialize file watcher object
                filewatchers[i] = new FileSystemWatcher(submissions_path + chall_names[i] + "\\", chall_names[i] + ".txt");
                filewatchers[i].EnableRaisingEvents = true;
                // set notification event
                filewatchers[i].NotifyFilter = NotifyFilters.FileName;
                // register event handler that
                // gets called when the file is created
                filewatchers[i].Created += new FileSystemEventHandler(OnCreate);
                // register a handler that gets called if the
                // challenge file watcher needs to report an error
                filewatchers[i].Error += new ErrorEventHandler(OnError);
            }
            LogEvent("The FileSystemWatcher has been started on these files: \n"
               submissions_path + chall_names[0] + "\\" + chall_names[0] + ".txt,\n"
               submissions_path + chall_names[1] + "\\" + chall_names[1] + ".txt,\n"
               submissions_path + chall_names[2] + "\\" + chall_names[2] + ".txt,\n"
               submissions_path + chall_names[3] + "\\" + chall_names[3] + ".txt,\n"
               submissions_path + chall_names[4] + "\\" + chall_names[4] + ".txt, and:\n"
               submissions_path + chall_names[5] + "\\" + chall_names[5] + ".txt.");
        }

        protected override void OnStop()
        {
            base.OnStop();
            // do nothing
        }

        public void Start()
        {
            OnStart(new string[0]);
        }

        public void Stop()
        {
            OnStop();
        }
    }
}

