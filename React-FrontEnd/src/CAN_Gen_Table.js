import * as React from 'react';
import { DataGrid } from '@mui/x-data-grid';
import {
  randomCreatedDate,
  randomTraderName,
  randomUpdatedDate,
} from '@mui/x-data-grid-generator';

export default function CAN_Gen_Table() {
  return (
    <div style={{ height: 600 }}>
      <DataGrid rows={rows} columns={columns} />
    </div>
  );
}

const columns = [
  { field: 'thread', headerName: 'Thread', width: 120, type: 'number', editable: true  },
  { field: 'count', headerName: 'Count', type: 'number', editable: true },
  { field: 'index', headerName: 'Index', type: 'number', editable: true },
  { field: 'send', headerName: 'Send', type: 'number', editable: true },
  { field: 'channel', headerName: 'Channel', type: 'number', editable: true },
  { field: 'period', headerName: 'Period', type: 'number', editable: true },
  { field: 'restart', headerName: 'Restart', type: 'number', editable: true },
  { field: 'total', headerName: 'Total', type: 'number', editable: true },
  { field: 'tx_count', headerName: 'TX Count', type: 'number', editable: true },
  { field: 'can_id', headerName: 'CAN ID', editable: true },
  { field: 'dlc', headerName: 'DLC', type: 'number', editable: true },
  { field: 'b0', headerName: 'B0', type: 'number', editable: true },
  { field: 'b1', headerName: 'B1', type: 'number', editable: true },
  { field: 'b2', headerName: 'B2', type: 'number', editable: true },
  { field: 'b3', headerName: 'B3', type: 'number', editable: true },
  { field: 'b4', headerName: 'B4', type: 'number', editable: true },
  { field: 'b5', headerName: 'B5', type: 'number', editable: true },
  { field: 'b6', headerName: 'B6', type: 'number', editable: true },
  { field: 'b7', headerName: 'B7', type: 'number', editable: true },
  { field: 'thread_name', headerName: 'Thread Label', editable: true },

//   { 
//     field: 'dateCreated',
//     headerName: 'Date Created',
//     type: 'date',
//     width: 180,
//     editable: true,
//   },
//   {
//     field: 'lastLogin',
//     headerName: 'Last Login',
//     type: 'dateTime',
//     width: 220,
//     editable: true,
//   },
];

const rows = [
  {
    id: 1,
    name: "test_name",
    age: 25,
    dateCreated: randomCreatedDate(),
    lastLogin: randomUpdatedDate(),
  }
];
