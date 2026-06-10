(* ::Package:: *)

(* ::Text:: *)
(*Alejandro Aviles (avilescervantes@gmail.com)*)
(*Extrapolation modules:*)
(**)
(*Linear extrapolation in log-log*)


LinearRegression[inputxy_]:=Module[{xm,ym,m,b,Npts,out},
xm=Mean[inputxy[[All,1]]];
ym=Mean[inputxy[[All,2]]];
Npts=Length@inputxy;
m=Sum[(inputxy[[i,1]]-xm)(inputxy[[i,2]]-ym),{i,1,Npts}]/Sum[(inputxy[[i,1]]-xm)^2,{i,1,Npts}];
b=ym - m * xm;
out={m,b}
];

Extrapolate[inputxy_,outputx_]:=Module[{m,b,outxy},
{m,b}=LinearRegression[inputxy];
outxy=Table[{outputx[[i]],m*outputx[[i]]+b},{i,1,Length@outputx}];
outxy
];

ExtrapolateHighkLogLog[inputT_,kcutmax_,kmax_]:=Module[
{inputcutT,tableToExtT,listToExtT,logklist,delta,lastk,sign,logextT,output},
inputcutT=Select[inputT,#[[1]]<=kcutmax&];
tableToExtT=Take[inputcutT[[All]],-6];
listToExtT=tableToExtT[[All,1]];
delta=Log10[listToExtT][[3]]-Log10[listToExtT][[2]];
lastk=Last[Log10[listToExtT]];
logklist={};
While[lastk<= Log10[kmax],AppendTo[logklist,lastk+delta];lastk=Last[logklist]];
sign=Sign[tableToExtT[[2,2]]];
tableToExtT=Log10[Abs[tableToExtT]];
logextT=Extrapolate[tableToExtT,logklist];
output=Transpose[{10^logextT[[All,1]],sign*10^logextT[[All,2]]}];
output=Join[inputcutT,output];
output
];

ExtrapolateLowkLogLog[inputT_,kcutmin_,kmin_]:=Module[
{inputcutT,tableToExtT,listToExtT,logklist,delta,firstk,sign,logextT,output},
inputcutT=Select[inputT,#[[1]]>kcutmin&];
tableToExtT=Take[inputcutT[[All]],5];
listToExtT=tableToExtT[[All,1]];
delta=Log10[listToExtT][[3]]-Log10[listToExtT][[2]];
firstk=First[Log10[listToExtT]];
logklist={};
While[firstk> Log10[kmin],PrependTo[logklist,firstk-delta];firstk=First[logklist]];
sign=Sign[tableToExtT[[2,2]]];
tableToExtT=Log10[Abs[tableToExtT]];
logextT=Extrapolate[tableToExtT,logklist];
output=Transpose[{10^logextT[[All,1]],sign*10^logextT[[All,2]]}];
output=Join[output,inputcutT];
output
];

ExtrapolatekLogLog[inputT_,kcutmin_,kmin_,kcutmax_,kmax_]:=Module[
{tempT,output},
output=ExtrapolateLowkLogLog[ExtrapolateHighkLogLog[inputT,kcutmax,kmax],kcutmin,kmin];
output
];
