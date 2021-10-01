import * as React from "react";
import { useState, useCallback } from "react";
import { styled } from "@mui/material/styles";
// import Box from "@mui/material/Box";
import Paper from "@mui/material/Paper";
// import Grid from "@mui/material/Grid";
import TextField from "@mui/material/TextField";
import Stack from "@mui/material/Stack";
import { Switch, Typography } from "@mui/material";
import { useEffect } from "react";
// import { typography } from "@mui/system";

const Item = styled(Paper)(({ theme }) => ({
  ...theme.typography.body1,
  padding: theme.spacing(1),
  textAlign: "center",
  color: theme.palette.text.secondary,
}));

export default function Pot(props) {
  const ChangeHandler = useCallback(({ target: { name, value } }) => {
    console.log("Chandler Inputs: ",name, value);
    props.setPWMSwitch(name, value);
    
  });

  const DCChangeHandler = useCallback(({ target: { name, value } }) => {
    console.log("DC Handler Inputs: ",name, value);
    props.setPWMState(name, value);
    
  });

  function setPWMDuty(name, duty) {
    console.log("Props",props);

    console.log("Input of setPWMState",name,duty);
    console.log("State:",props.data.pwm);
    let items = props.data.pwm;
    console.log("Items:",items);
    let item = { ...items[name] };
    console.log("Item:",item);
    item.duty.value = 48;
    items[name] = item;
    // props.setPWMState(items);
  }


  useEffect(() => {
    console.log(props.data);
  });
  const DC_desc = "Duty Cycle";
  return (
    <div>
    <Stack direction="row" spacing={1} alignItems="top">
      <Typography style={{"margin-top":10}}>Test1</Typography>
      <Switch name="pwm1" onChange={ChangeHandler} checked={props.data.sw.value} />
      <TextField
        name="pwm1"
        size="small"
        id="outlined-basic"
        error={props.data.duty.error}
        label={DC_desc}
        helperText={
        props.data.duty.helperText +
          " " +
          props.data.duty.value
        }
        onChange={DCChangeHandler}
        type="number"
        value={props.data.duty.value}

      />
      <TextField
        name="pwm1"
        size="small"
        id="outlined-basic"
        label="Frequency"
        error={props.data.freq.error}
        helperText={
            props.data.freq.helperText +
              " " +
              props.data.freq.value
            }
        value={props.data.freq.value}
      />
    </Stack>
    </div>
   
  );
}
