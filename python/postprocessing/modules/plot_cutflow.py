import ROOT

# Open the ROOT file
input_file = ROOT.TFile("cutflow.root", "READ")

cutflow_hist = input_file.Get("cutflow")
canvas = ROOT.TCanvas("canvas", "Cutflow Histogram", 800, 600)

cutflow_hist.Draw("HIST")

cutflow_hist.SetTitle("Cutflow;Cuts;Number of Events")
cutflow_hist.GetXaxis().SetLabelSize(0.04)
cutflow_hist.GetYaxis().SetTitleOffset(1.2)

canvas.SaveAs("cutflow.png")
input_file.Close()
